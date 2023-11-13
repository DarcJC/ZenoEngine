// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZenoEngine.h"

#include "HttpModule.h"

#define LOCTEXT_NAMESPACE "FZenoEngineModule"

void FZenoEngineModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	using namespace zpp::bits::literals;

	auto Handler = [] (auto Result)
	{
		if (zpp::bits::success(Result))
		{
			int32 i = Result.or_throw();
			UE_LOG(LogTemp, Warning, TEXT("WTF %d"), i);
		}
	};
	REQUEST_REMOTE_ZENO_GENERIC(roads::service::rpc, "TestFunc"_sha256_int, Handler, 123);
}

void FZenoEngineModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FZenoEngineModule::AsyncCallZenoApi(const TArray<uint8>& Data, const FHttpRequestCompleteDelegate& InCallback)
{
	FHttpModule& HttpModule = FHttpModule::Get();
	auto NewRequest = HttpModule.CreateRequest();
	NewRequest->SetVerb("POST");
	NewRequest->SetURL("http://localhost:19990/v0/zeno");
	NewRequest->SetHeader("Content-Type", "application/x-zeno");
	NewRequest->SetTimeout(5.0f);
	NewRequest->SetContent(Data);
	NewRequest->OnProcessRequestComplete() = InCallback;
	NewRequest->ProcessRequest();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FZenoEngineModule, ZenoEngine)
