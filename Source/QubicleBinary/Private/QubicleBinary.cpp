// Copyright 2015 afuzzyllama. All Rights Reserved.

#include "QubicleBinaryPrivatePCH.h"

DEFINE_LOG_CATEGORY(LogQubicleBinary);

class FQubicleBinary : public IQubicleBinary
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE( FQubicleBinary, QubicleBinary )



void FQubicleBinary::StartupModule()
{
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
}


void FQubicleBinary::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}



