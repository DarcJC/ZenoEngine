// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZenoEngine.h"

#include "HttpModule.h"
#include "JsonObjectConverter.h"

#define LOCTEXT_NAMESPACE "FZenoEngineModule"

void FZenoEngineModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	using namespace zpp::bits::literals;
	TZenoSimplePromise<FZenoRPCContext> ContextPromise = REQUEST_REMOTE_ZENO_GENERIC(roads::service::rpc, "TestFunc"_sha256_int, 123);
	ContextPromise.Then([] (FZenoRPCContext& Context)
	{
		GET_RPC_CLIENT_INSTANCE(Context.Buffer);
		FZenoTerrainData i = Client.response<"TestFunc"_sha256_int>().or_throw();
		UE_LOG(LogTemp, Warning, TEXT("WTF %lld"), i.Data.InternalData.size());
		FString JSON;
		FJsonObjectConverter::UStructToJsonObjectString<FZenoTerrainData>(i, JSON);
		UE_LOG(LogTemp, Warning, TEXT("WTF %s"), *JSON);
	});
}

void FZenoEngineModule::ShutdownModule()
{
}

TSharedRef<TPromise<FZenoRPCContext>> FZenoEngineModule::CreateContextPromise()
{
	return MakeShared<TPromise<FZenoRPCContext>>();
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
