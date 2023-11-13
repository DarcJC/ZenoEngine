// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "shared_server.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Modules/ModuleManager.h"
#include <functional>

#define REQUEST_REMOTE_ZENO_GENERIC(RPCType, Id, ...) \
	[] () -> TZenoSimplePromise<FZenoRPCContext> { \
	std::vector<std::byte> Buffer; \
	zpp::bits::in InWrapper{Buffer}; \
	zpp::bits::out OutWrapper{Buffer}; \
	RPCType::client Client { InWrapper, OutWrapper }; \
	Client.request<Id>(__VA_ARGS__).or_throw(); \
	TArray<uint8> TransformedBuffer; \
	TransformedBuffer.AddUninitialized(Buffer.size()); \
	FMemory::Memcpy(TransformedBuffer.GetData(), Buffer.data(), TransformedBuffer.Num()); \
	TZenoSimplePromise<FZenoRPCContext> ContextPromise; \
	AsyncCallZenoApi(TransformedBuffer, FHttpRequestCompleteDelegate::CreateLambda([ContextPromise] (FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectSuccessfully) {\
		if (bConnectSuccessfully && Response) { \
			TArray<uint8> RespData = Response->GetContent(); \
			std::vector<uint8> TransformedData;\
			TransformedData.resize(RespData.Num());\
			FMemory::Memcpy(TransformedData.data(), RespData.GetData(), TransformedData.size());\
			zpp::bits::in InWrapperResp{TransformedData};\
			zpp::bits::out OutWrapperResp{TransformedData};\
			FZenoRPCContext Context { std::move(TransformedData) };\
			ContextPromise.SetResult(Context);\
		}\
	})); \
	return ContextPromise;\
	}();

#define GET_RPC_CLIENT_INSTANCE(InBuffer) \
		zpp::bits::in In_ { InBuffer };\
		zpp::bits::out Out_ { InBuffer };\
		roads::service::rpc::client Client { In_, Out_ };\

struct FZenoRPCContext
{
	std::vector<uint8> Buffer;
};

template <typename ResultType>
struct TZenoSimplePromise
{
	using FPendingFunctionType = TFunction<void(ResultType&)>;
	
	void SetResult(const ResultType& InResult) const
	{
		Result = MakeShared<FZenoRPCContext>(InResult);
		*bIsFullFiled = true;
		while (!InternalQueue->IsEmpty() && Result.IsValid())
		{
			FPendingFunctionType FunctionToCall;
			InternalQueue->Dequeue(FunctionToCall);
			FunctionToCall(*Result);
		}
	}

	void Then(FPendingFunctionType InFunction) const
	{
		if (*bIsFullFiled && Result.IsValid())
		{
			InFunction(*Result);
		}
		else
		{
			InternalQueue->Enqueue(InFunction);
		}
	}

private:
	mutable TSharedRef<bool> bIsFullFiled = MakeShared<bool>(false);
	mutable TSharedPtr<ResultType> Result;

	mutable TSharedRef<TQueue<FPendingFunctionType>> InternalQueue = MakeShared<TQueue<FPendingFunctionType>>();
};

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

	static TSharedRef<TPromise<FZenoRPCContext>> CreateContextPromise();

protected:
	static void AsyncCallZenoApi(const TArray<uint8>& Data, const FHttpRequestCompleteDelegate& InCallback);
};
