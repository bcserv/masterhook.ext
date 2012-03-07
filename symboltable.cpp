
#include "smsdk_ext.h"
#include "symboltable.h"

#if defined PLATFORM_POSIX
SourceHook::CVector<LibSymbolTable *> m_SymTables;
#endif

void *ResolveDemangledSymbol(void* handle, const char* symbol, int& type, char* argsBuffer, unsigned int len) {
#ifdef PLATFORM_WINDOWS

	return GetProcAddress((HMODULE)handle, symbol);
	
#elif defined PLATFORM_LINUX

	struct link_map* dlmap;
	struct stat dlstat;
	int dlfile;
	uintptr_t map_base;
	Elf32_Ehdr* file_hdr;
	Elf32_Shdr* sections, * shstrtab_hdr, * symtab_hdr, * strtab_hdr;
	Elf32_Sym* symtab;
	const char* shstrtab, * strtab;
	uint16_t section_count;
	uint32_t symbol_count;
	LibSymbolTable* libtable;
	SymbolTable* table;
	Symbol* symbol_entry = NULL;

	dlmap = (struct link_map*)handle;
	symtab_hdr = NULL;
	strtab_hdr = NULL;
	table = NULL;
	
	/* See if we already have a symbol table for this library */
	/*for (size_t i = 0; i < m_SymTables.size(); i++)
	{
		libtable = m_SymTables[i];
		if (libtable->lib_base == dlmap->l_addr)
		{
			table = &libtable->table;
			break;
		}
	}*/

	/* If we don't have a symbol table for this library, then create one */
	if (table == NULL)
	{
		libtable = new LibSymbolTable();
		libtable->table.Initialize();
		libtable->lib_base = dlmap->l_addr;
		libtable->last_pos = 0;
		table = &libtable->table;
		m_SymTables.push_back(libtable);
	}

	// TODO Make this work with demangled symbols ?
	/* See if the symbol is already cached in our table */
	/*symbol_entry = table->FindSymbol(symbol, strlen(symbol));
	if (symbol_entry != NULL)
	{
		return symbol_entry->address;
	}*/

	/* If symbol isn't in our table, then we have open the actual library */
	dlfile = open(dlmap->l_name, O_RDONLY);
	if (dlfile == -1 || fstat(dlfile, &dlstat) == -1)
	{
		close(dlfile);
		return NULL;
	}

	/* Map library file into memory */
	file_hdr = (Elf32_Ehdr *)mmap(NULL, dlstat.st_size, PROT_READ, MAP_PRIVATE, dlfile, 0);
	map_base = (uintptr_t)file_hdr;
	if (file_hdr == MAP_FAILED)
	{
		close(dlfile);
		return NULL;
	}
	close(dlfile);

	if (file_hdr->e_shoff == 0 || file_hdr->e_shstrndx == SHN_UNDEF)
	{
		munmap(file_hdr, dlstat.st_size);
		return NULL;
	}

	sections = (Elf32_Shdr *)(map_base + file_hdr->e_shoff);
	section_count = file_hdr->e_shnum;
	/* Get ELF section header string table */
	shstrtab_hdr = &sections[file_hdr->e_shstrndx];
	shstrtab = (const char *)(map_base + shstrtab_hdr->sh_offset);

	/* Iterate sections while looking for ELF symbol table and string table */
	for (uint16_t i = 0; i < section_count; i++)
	{
		Elf32_Shdr &hdr = sections[i];
		const char *section_name = shstrtab + hdr.sh_name;

		if (strcmp(section_name, ".symtab") == 0)
		{
			symtab_hdr = &hdr;
		}
		else if (strcmp(section_name, ".strtab") == 0)
		{
			strtab_hdr = &hdr;
		}
	}

	/* Uh oh, we don't have a symbol table or a string table */
	if (symtab_hdr == NULL || strtab_hdr == NULL)
	{
		munmap(file_hdr, dlstat.st_size);
		return NULL;
	}

	symtab = (Elf32_Sym *)(map_base + symtab_hdr->sh_offset);
	strtab = (const char *)(map_base + strtab_hdr->sh_offset);
	symbol_count = symtab_hdr->sh_size / symtab_hdr->sh_entsize;

	/* Iterate symbol table starting from the position we were at last time */
	for (uint32_t i = libtable->last_pos; i < symbol_count; i++)
	{
		Elf32_Sym& sym = symtab[i];
		unsigned char sym_type = ELF32_ST_TYPE(sym.st_info);
		const char* sym_name = strtab + sym.st_name;
		Symbol* cur_sym;

		/* Skip symbols that are undefined or do not refer to functions or objects */
		if (sym.st_shndx == SHN_UNDEF || (sym_type != STT_FUNC && sym_type != STT_OBJECT))
		{
			continue;
		}

		char *name_demangled = NULL;
		int status;
		size_t length = 0;
		name_demangled = abi::__cxa_demangle(sym_name, NULL, &length, &status);

		if (status != 0) {
			continue;
		}

		//META_CONPRINTF("[MASTERHOOK] Checkpoint: 1\n");
		//META_CONPRINTF("[MASTERHOOK] Checkpoint: 1.1 \"%s\"\n", name_demangled);
		//META_CONPRINTF("[MASTERHOOK] Checkpoint: 1.2\n");

		char* brace_open = NULL;
		if (sym_type == STT_FUNC && strrchr(symbol, '(')  == NULL) { // Is Function and passed symbol doesn't have an arglist

			// Let's strip off the arglist as we don't want to compare with that
			brace_open = strrchr(name_demangled, '(');

			if (brace_open != NULL) {
				brace_open[0] = '\0';
			}
		}

		/* Caching symbols as we go along */
		cur_sym = table->InternSymbol(name_demangled, strlen(name_demangled), (void *)(dlmap->l_addr + sym.st_value));
		if (strcmp(name_demangled, symbol) == 0) {

			if (brace_open != NULL) {
				char* brace_close = strrchr(brace_open+1, ')');

				if (brace_close != NULL) {
					brace_close[0] = '\0';
				}

				if (length > strlen(brace_open+1)) {
					brace_close[len] = '\0';
				}
				//META_CONPRINTF("[MASTERHOOK] Checkpoint: 2 \"%s\" d:%d\n", brace_open+1, len);
				memmove(argsBuffer, brace_open+1, len);
				argsBuffer[len] = '\0';
			}

			symbol_entry = cur_sym;
			libtable->last_pos = ++i;
			type = STT_FUNC;
			free(name_demangled);
			break;
		}

		//META_CONPRINTF("[MASTERHOOK] Checkpoint: 3 \"%s\"\n", name_demangled);
		free(name_demangled);
		//META_CONPRINTF("[MASTERHOOK] Checkpoint: 4\n");
	}
#ifdef DEBUG
	//META_CONPRINTF("[MASTERHOOK] Checkpoint: 5 - End of Symbol Table\n");
#endif

	munmap(file_hdr, dlstat.st_size);
#ifdef DEBUG
	META_CONPRINTF("[MASTERHOOK] type: %d\n", type);
#endif
	return symbol_entry ? symbol_entry->address : NULL;

#elif defined PLATFORM_APPLE
	
	uintptr_t dlbase, linkedit_addr;
	uint32_t image_count;
	struct mach_header *file_hdr;
	struct load_command *loadcmds;
	struct segment_command *linkedit_hdr;
	struct symtab_command *symtab_hdr;
	struct nlist *symtab;
	const char *strtab;
	uint32_t loadcmd_count;
	uint32_t symbol_count;
	LibSymbolTable *libtable;
	SymbolTable *table;
	Symbol *symbol_entry;
	
	dlbase = 0;
	image_count = m_ImageList->infoArrayCount;
	linkedit_hdr = NULL;
	symtab_hdr = NULL;
	table = NULL;
	
	/* Loop through mach-o images in process.
	 * We can skip index 0 since that is just the executable.
	 */
	for (uint32_t i = 1; i < image_count; i++)
	{
		const struct dyld_image_info &info = m_ImageList->infoArray[i];
		
		/* "Load" each one until we get a matching handle */
		void *h = dlopen(info.imageFilePath, RTLD_NOLOAD);
		if (h == handle)
		{
			dlbase = (uintptr_t)info.imageLoadAddress;
			dlclose(h);
			break;
		}
		
		dlclose(h);
	}
	
	if (!dlbase)
	{
		/* Uh oh, we couldn't find a matching handle */
		return NULL;
	}
	
	/* See if we already have a symbol table for this library */
	for (size_t i = 0; i < m_SymTables.size(); i++)
	{
		libtable = m_SymTables[i];
		if (libtable->lib_base == dlbase)
		{
			table = &libtable->table;
			break;
		}
	}
	
	/* If we don't have a symbol table for this library, then create one */
	if (table == NULL)
	{
		libtable = new LibSymbolTable();
		libtable->table.Initialize();
		libtable->lib_base = dlbase;
		libtable->last_pos = 0;
		table = &libtable->table;
		m_SymTables.push_back(libtable);
	}
	
	/* See if the symbol is already cached in our table */
	symbol_entry = table->FindSymbol(symbol, strlen(symbol));
	if (symbol_entry != NULL)
	{
		return symbol_entry->address;
	}
	
	/* If symbol isn't in our table, then we have to locate it in memory */
	
	file_hdr = (struct mach_header *)dlbase;
	loadcmds = (struct load_command *)(dlbase + sizeof(struct mach_header));
	loadcmd_count = file_hdr->ncmds;
	
	/* Loop through load commands until we find the ones for the symbol table */
	for (uint32_t i = 0; i < loadcmd_count; i++)
	{
		if (loadcmds->cmd == LC_SEGMENT && !linkedit_hdr)
		{
			struct segment_command *seg = (struct segment_command *)loadcmds;
			if (strcmp(seg->segname, "__LINKEDIT") == 0)
			{
				linkedit_hdr = seg;
				if (symtab_hdr)
				{
					break;
				}
			}
		}
		else if (loadcmds->cmd == LC_SYMTAB)
		{
			symtab_hdr = (struct symtab_command *)loadcmds;
			if (linkedit_hdr)
			{
				break;
			}
		}

		/* Load commands are not of a fixed size which is why we add the size */
		loadcmds = (struct load_command *)((uintptr_t)loadcmds + loadcmds->cmdsize);
	}
	
	if (!linkedit_hdr || !symtab_hdr || !symtab_hdr->symoff || !symtab_hdr->stroff)
	{
		/* Uh oh, no symbol table */
		return NULL;
	}

	linkedit_addr = dlbase + linkedit_hdr->vmaddr;
	symtab = (struct nlist *)(linkedit_addr + symtab_hdr->symoff - linkedit_hdr->fileoff);
	strtab = (const char *)(linkedit_addr + symtab_hdr->stroff - linkedit_hdr->fileoff);
	symbol_count = symtab_hdr->nsyms;
	
	/* Iterate symbol table starting from the position we were at last time */
	for (uint32_t i = libtable->last_pos; i < symbol_count; i++)
	{
		struct nlist &sym = symtab[i];
		/* Ignore the prepended underscore on all symbols, so +1 here */
		const char *sym_name = strtab + sym.n_un.n_strx + 1;
		Symbol *cur_sym;
		
		/* Skip symbols that are undefined */
		if (sym.n_sect == NO_SECT)
		{
			continue;
		}
		
		/* Caching symbols as we go along */
		cur_sym = table->InternSymbol(sym_name, strlen(sym_name), (void *)(dlbase + sym.n_value));
		if (strcmp(symbol, sym_name) == 0)
		{
			symbol_entry = cur_sym;
			libtable->last_pos = ++i;
			break;
		}
	}
	
	return symbol_entry ? symbol_entry->address : NULL;

#endif
}

/* Given Elf header, Elf_Scn, and Elf32_Shdr 
 * print out the symbol table 
 */
void *GetSymbolAdress(const char *filename, const char *symbol) {

#ifdef PLATFORM_POSIX

	int fd;
	Elf *elf;
	GElf_Ehdr elfhdr;

	if (elf_version(EV_CURRENT) == EV_NONE) {
		META_CONPRINTF("ELF library is out of date, Quitting\n");
	}

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		META_CONPRINTF("[Masterhook] Unable to open \"%s\"\n", filename);
	}

	elf = elf_begin(fd, ELF_C_READ, NULL);
	if (gelf_getehdr(elf, &elfhdr) == 0)	{
		META_CONPRINTF("Cannot read elf header. This usually means that %s is not a valid elf file\n", filename);
	}

	Elf_Scn* section = 0;
	Elf32_Shdr *shdr = NULL;
	bool symtabFound = false;
	while ((section = elf_nextscn(elf, section)) != 0) {
		if ((shdr = elf32_getshdr (section)) != 0) {
			if (shdr->sh_type == SHT_SYMTAB) {
				symtabFound = true;
				break;
			}
		}
	}

	if (shdr == NULL || !symtabFound) {
		return NULL;
	}

	Elf_Scn *scn = section;

	Elf_Data *data;
	char *name;
	data = 0;
	int number = 0;
	if ((data = elf_getdata(scn, data)) == 0 || data->d_size == 0){
		META_CONPRINTF("Section had no data!\n");
		return NULL;
	}

	Elf32_Sym *esym = (Elf32_Sym*) data->d_buf;
	Elf32_Sym *lastsym = (Elf32_Sym*) ((char*) data->d_buf + data->d_size);
	for (; esym < lastsym; esym++) {
		if ((esym->st_value == 0) || (ELF32_ST_BIND(esym->st_info) == STB_WEAK) || (ELF32_ST_BIND(esym->st_info) == STB_NUM) || (ELF32_ST_TYPE(esym->st_info) != STT_FUNC)) {
			continue;
		}

		name = elf_strptr(elf, shdr->sh_link , (size_t)esym->st_name);
		if(!name){
			META_CONPRINTF("%s\n", elf_errmsg(elf_errno()));
			return NULL;
		}

		char *name_demangled = 0;
		int* status;
		name_demangled = abi::__cxa_demangle(name, 0, 0, status);

		#ifdef DEBUG
		META_CONPRINTF("%d: Name: %s == %s Address: 0x%x\n", number++, name_demangled, symbol, esym->st_value);
#endif

		if (name_demangled != NULL) {
			if (strstr(name_demangled, symbol)) {
				return (void*)esym->st_value;
			}
		}

		//free(name_demangled);
	}
#endif

	return NULL;
}
