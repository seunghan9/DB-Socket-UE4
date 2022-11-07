// Fill out your copyright notice in the Description page of Project Settings.


#include "MutiThread.h"
#include "MyActor.h"
#include "DataGameModeBase.h"

#include "Sockets.h"
#include "Networking.h"
#include "SocketSubsystem.h"

#define PORT	13333
#define PACKED_SIZE 1024
#define SERVER_DOMAIN	"tenth.iptime.org"

//"218.145.204.176"
#pragma region Main Thread Code
MultiThread::MultiThread()
{
	Thread = FRunnableThread::Create(this, TEXT("SocketServer"));
}

MultiThread::MultiThread(ADataGameModeBase* a)
{
	MyGameModeBase = a;
	Thread = FRunnableThread::Create(this, TEXT("SocketServer"));
}


MultiThread::~MultiThread()
{
	if (Socket->Close())
	{
		UE_LOG(LogTemp, Warning, TEXT("Close Socket Error"));
	}
	if (Thread)
	{
		Thread->Kill(true);
		delete Thread;
	}
}
#pragma endregion


bool MultiThread::Init()
{
	UE_LOG(LogTemp, Warning, TEXT("My custom thread has been initialized"));

		return true;
}


uint32 MultiThread::Run()
{

	

	// ���� ����
	// ���� Ÿ�԰� ������ ���ڷ� �ִ´�.TEXT("Stream")�� �ָ� TCP ���������� ����ϰڴٴ� ���̴�. (UDP�� TEXT("DGram")�� ���ڷ� �ָ� �ȴ�.)
	//�� ��° ���� TEXT("Client Socket") �� ����� �̸��̴�.�˱� ���� �̸����� ����������.
	// �� ��° ���ڴ� UDP�� true TCP�� flase�̴�.

	Socket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(TEXT("Stream"), TEXT("Client Socket"));

	// ��Ʈ�� ������ ��� Ŭ����
	// FInternetAddr�� ��Ʈ��ũ ������ �����ϰ�, ��Ŭ�������� ��Ʈ��ũ ����Ʈ�� ������ �ȴ�.
	TSharedRef<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

	FString ReturnValue;
	ISocketSubsystem* const SocketSubSystem = ISocketSubsystem::Get();
	uint32 OutIP = 0;

	if (ensure(SocketSubSystem))
	{
		auto ResolveInfo = SocketSubSystem->GetHostByName(SERVER_DOMAIN);
		while (true)
		{
			if (ResolveInfo->IsComplete())
			{
				break;
			}
		}
			if (ResolveInfo->IsComplete() && ResolveInfo->GetErrorCode() == 0)
			{
				const FInternetAddr* Addr = &ResolveInfo->GetResolvedAddress();
				Addr->GetIp(OutIP);
				ReturnValue = Addr->ToString(false);
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("Connect fail ErrorCode : %d"), ResolveInfo->GetErrorCode());
			}
	}

	int32 port = PORT;

	addr->SetIp(OutIP);
	addr->SetPort(port);

	bool isConnected = Socket->Connect(*addr);

	Package ReceivePack;

	if (isConnected)
	{
		MyGameModeBase->IsConnected = true;
		UE_LOG(LogTemp, Log, TEXT("Connect Success"));
		while (bRunThread)
		{

			FPlatformProcess::Sleep(0.1f);

			uint8 len;
			int32	temp1 = 0;
			Socket->Recv(&len, 4, temp1);

			uint8	Buffer[1024] = { 0 };
			int32	temp = 0;
			Socket->Recv(Buffer, len, temp);

			ReceivePack = *(Package*)Buffer;

			
			MyGameModeBase->ReceivePack = ReceivePack;

			UE_LOG(LogTemp, Log, TEXT("Header : %d X : %d Z : %d"), MyGameModeBase->ReceivePack.Header, MyGameModeBase->ReceivePack.X, MyGameModeBase->ReceivePack.Z);


		}
		
		return 0;

	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Connect fail"));

		return 0;
	}
}

void MultiThread::Stop()
{
	if (!Thread)
	{
		Thread->Kill();
		delete Thread;
	}
	
	bRunThread = false;
}
