#include "func_types.h"
#include "natives.h"
#include <IBinTools.h>

class Masterhook_TypeConverter
{

public:
	static SourceMod::CallConvention ToSourcemodCallConvention(eCallConv conv);
	static SourceMod::PassInfo ArgDescToSourcemodPassInfo();

	size_t ToBinParam(MasterhookDataType type, unsigned int flags, PassInfo *info, bool &needs_extra);
	bool Decode(IPluginContext *pContext, cell_t param, const PassInfo *data, const MasterhookDataType  dataType, void *_buffer);
};
