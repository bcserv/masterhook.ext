#include "typeconverter.h"
#include <IBinTools.h>

SourceMod::CallConvention Masterhook_TypeConverter::ToSourcemodCallConvention(eCallConv conv)
{
	switch (conv) {
		case CONV_THISCALL: {
			return SourceMod::CallConv_ThisCall;
		}
	}

	return SourceMod::CallConv_Cdecl;
}

/**
 * For object pointers, the data looks like this instead:
 * 4 bytes: POINTER TO LATER
 * + bytes: Object internal data
 *
 * We use the virtual stack as extra fake stack space and create a temp object.
 * If these objects had destructors, we'd need to fake destroy toom of course.
 * Of course, BinTools only reads the first four bytes and passes the pointer.
 */

size_t Masterhook_TypeConverter::ToBinParam(MasterhookDataType type,
						  unsigned int flags,
						  PassInfo *info,
						  bool &needs_extra)
{
	needs_extra = false;
	switch (type)
	{
		case MhDataType_Vector:
		{
			size_t mySize = sizeof(Vector *);

			info->type = PassType_Basic;
			info->flags = flags;
			info->size = sizeof(Vector *);
			mySize = sizeof(Vector);
			needs_extra = true;

			return mySize;
		}
		case MhDataType_QAngle: {
			info->type = PassType_Basic;
			info->flags = flags;
			info->size = sizeof(QAngle *);
			needs_extra = true;
			return sizeof(QAngle);
		}
		case MhDataType_CBaseEntity:
		case MhDataType_Edict:
		case MhDataType_Char_Ptr:
		case MhDataType_Void: {
			info->type = PassType_Basic;
			info->flags = flags;
			info->size = sizeof(void *);
			return sizeof(void *);
		}
		case MhDataType_Char: {
			info->type = PassType_Basic;
			info->flags = flags;
			info->size = sizeof(char);
			return sizeof(char);
		}
		case MhDataType_Int8: {
			info->type = PassType_Basic;
			info->flags = flags;
			info->size = sizeof(byte);
			return sizeof(byte);
		}
		case MhDataType_Int8_Ptr: {
			info->type = PassType_Basic;
			info->flags = flags;
			needs_extra = true;
			info->size = sizeof(byte *);
			return sizeof(byte *) + sizeof(byte);
		}
		case MhDataType_Int16: {
			info->type = PassType_Basic;
			info->flags = flags;
			info->size = sizeof(short);
			return sizeof(short);
		}
		case MhDataType_Int16_Ptr: {
			info->type = PassType_Basic;
			info->flags = flags;
			needs_extra = true;
			info->size = sizeof(short *);
			return sizeof(short *) + sizeof(short);
		}
		case MhDataType_Int32: {
			info->type = PassType_Basic;
			info->flags = flags;
			info->size = sizeof(int);
			return sizeof(int);
		}
		case MhDataType_Int32_Ptr: {
			info->type = PassType_Basic;
			info->flags = flags;
			needs_extra = true;
			info->size = sizeof(int *);
			return sizeof(int *) + sizeof(int);
		}
		case MhDataType_Int64: {
			info->type = PassType_Basic;
			info->flags = flags;
			info->size = sizeof(float);
			return sizeof(float);
		}
		case MhDataType_Int64_Ptr: {
			info->type = PassType_Basic;
			info->flags = flags;
			needs_extra = true;
			info->size = sizeof(int64 *);
			return sizeof(int64 *) + sizeof(int64);
		}
		case MhDataType_Float: {
				info->type = PassType_Float;
				info->size = sizeof(float);
				return sizeof(float);
		}
		case MhDataType_Float_Ptr: {
				needs_extra = true;
				info->type = PassType_Basic;
				info->size = sizeof(int64 *);
				return sizeof(int64 *) + sizeof(int64);
		}
		case MhDataType_Bool:
		{
			info->type = PassType_Basic;
			info->size = sizeof(bool);
			return sizeof(bool);
		}			
	}

	return 0;
}

bool Masterhook_TypeConverter::Decode(IPluginContext *pContext,
					  cell_t param,
					  const PassInfo *data,
					  const MasterhookDataType  dataType,
					  void *_buffer)
{
	void *buffer = (unsigned char *)_buffer + data->size;
	switch (dataType) {

		case MhDataType_Vector: {

			cell_t *addr;
			int err;
			err = pContext->LocalToPhysAddr(param, &addr);

			unsigned char *mem = (unsigned char *)buffer;
			/* Store the object in the next N bytes, and store
				* a pointer to that object right beforehand.
				*/
			Vector **realPtr = (Vector **)buffer;

			if (addr == pContext->GetNullRef(SP_NULL_VECTOR)) {
				*realPtr = NULL;
				return true;
			}
			else {
				//mem = (unsigned char *)_buffer + pCall->stackEnd + data->obj_offset;
				*realPtr = (Vector *)mem;
			}

			if (err != SP_ERROR_NONE) {
				pContext->ThrowNativeErrorEx(err, "Could not read plugin data");
				return false;
			}

			/* Use placement new to initialize the object cleanly
				* This has no destructor so we don't need to do 
				* DestroyValveParam() or something :]
				*/
			Vector *v = new (mem) Vector(
				sp_ctof(addr[0]),
				sp_ctof(addr[1]),
				sp_ctof(addr[2])
			);

			return true;
		}
		case MhDataType_QAngle: {

			cell_t *addr;
			int err;
			err = pContext->LocalToPhysAddr(param, &addr);

			unsigned char *mem = (unsigned char *)buffer;

				/* Store the object in the next N bytes, and store
					* a pointer to that object right beforehand.
					*/
				QAngle **realPtr = (QAngle **)buffer;

				if (addr == pContext->GetNullRef(SP_NULL_VECTOR)) {
					*realPtr = NULL;
					return true;
				}
				else {
					//mem = (unsigned char *)_buffer + pCall->stackEnd + data->obj_offset;
					*realPtr = (QAngle *)mem;
				}

			if (err != SP_ERROR_NONE) {
				pContext->ThrowNativeErrorEx(err, "Could not read plugin data");
				return false;
			}

			/* Use placement new to initialize the object cleanly
				* This has no destructor so we don't need to do 
				* DestroyValveParam() or something :]
				*/
			QAngle *v = new (mem) QAngle(
				sp_ctof(addr[0]),
				sp_ctof(addr[1]),
				sp_ctof(addr[2])
			);

			return true;
		}
		case MhDataType_CBaseEntity: {

			CBaseEntity *pEntity = NULL;

			int index = gamehelpers->ReferenceToIndex(param);
			if ((unsigned)index == INVALID_EHANDLE_INDEX && param != -1)
			{
				return false;
			}
			
			CBaseEntity **ebuf = (CBaseEntity **)buffer;
			*ebuf = pEntity;

			return true;
		}
		case MhDataType_Edict: {

			edict_t *pEdict;

			edict_t **ebuf = (edict_t **)buffer;
			*ebuf = pEdict;

			return true;
		}
		case MhDataType_Float:
		case MhDataType_Int8:
		case MhDataType_Int16:
		case MhDataType_Int32:
		case MhDataType_Int64: {

			*(cell_t *)buffer = param;
			return true;
		}
		case MhDataType_Bool: {

			*(bool *)buffer = param ? true : false;
			return true;
		}
		case MhDataType_Char_Ptr: {

			char *addr;
			pContext->LocalToString(param, &addr);
			*(char **)buffer = addr;
			return true;
		}
	}

	return false;
}
