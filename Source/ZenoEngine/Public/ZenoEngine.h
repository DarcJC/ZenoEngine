// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "shared_server.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Modules/ModuleManager.h"

#define REQUEST_REMOTE_ZENO_GENERIC(RPCType, Id, Callback, ...) \
	std::vector<std::byte> Buffer; \
	zpp::bits::in InWrapper{Buffer}; \
	zpp::bits::out OutWrapper{Buffer}; \
	RPCType::client Client { InWrapper, OutWrapper }; \
	Client.request<Id>(__VA_ARGS__).or_throw(); \
	TArray<uint8> TransformedBuffer; \
	TransformedBuffer.AddUninitialized(Buffer.size()); \
	FMemory::Memcpy(TransformedBuffer.GetData(), Buffer.data(), TransformedBuffer.Num()); \
	AsyncCallZenoApi(TransformedBuffer, FHttpRequestCompleteDelegate::CreateLambda([Callback] (FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectSuccessfully) {\
		if (bConnectSuccessfully && Response) { \
			TArray<uint8> RespData = Response->GetContent(); \
			std::vector<uint8> TransformedData;\
			TransformedData.resize(RespData.Num());\
			FMemory::Memcpy(TransformedData.data(), RespData.GetData(), TransformedData.size());\
			zpp::bits::in InWrapperResp{TransformedData};\
			zpp::bits::out OutWrapperResp{TransformedData};\
			RPCType::client Client2 { InWrapperResp, OutWrapperResp }; \
			Callback(Client2.response<Id>());\
		}\
	})); 

class FZenoEngineModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	template <typename RPCType, auto Id>
	void HandleHttpRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectSuccessfully)
	{
		if (bConnectSuccessfully && Response)
		{
			TArray<uint8> RespData = Response->GetContent();
			std::vector<uint8> TransformedData;
			TransformedData.resize(RespData.Num());
			FMemory::Memcpy(TransformedData.data(), RespData.GetData(), TransformedData.size());
			zpp::bits::in InWrapperResp{TransformedData};
			zpp::bits::out OutWrapperResp{TransformedData};
			typename RPCType::client RPCClientResp {InWrapperResp, OutWrapperResp};
			auto Result = RPCClientResp.template response<Id>();
			Callback(Result);
		}
	}

protected:
	static void AsyncCallZenoApi(const TArray<uint8>& Data, const FHttpRequestCompleteDelegate& InCallback);
};
