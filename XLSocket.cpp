﻿//安いだけじゃないソケット!!
// XLSocket.cpp: XLSocket クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////
#include "common.h"
#include <string>
#include <assert.h>
#include <time.h>
#include "XLException.h"
#include "XLSocket.h"


#if _MSC_VER
	//IPアドレスを取得するのに使う
	#include <iptypes.h>
	#include <iphlpapi.h>
	#pragma comment(lib, "ws2_32.lib")
	#pragma comment(lib, "IPHLPAPI.lib")
	#pragma comment(lib, "openssl.lib")

#else
	#include <sys/ioctl.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <net/if.h>
#endif


//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

XLSocket::XLSocket()
{
	this->Socket = INVALID_SOCKET;
	this->Connected = false;
	//SSL
	this->UseSSL = false;
	this->SSLContext = NULL;
	this->SSLhandle = NULL;
}

XLSocket::~XLSocket()
{
	if (this->Connected)
	{
		Shutdown();
	}
	if (this->Socket != INVALID_SOCKET)
	{
		Close();
	}
}

XLSocket::XLSocket(SOCKET inSocket)
{
	this->Socket = inSocket;
	this->Connected = true;
	this->UseSSL = false;

	this->SSLhandle = NULL;
	this->SSLContext = NULL;

}


void XLSocket::CreateLow( int type , int protocol ,int timeout)
{
	Close();
	 
	//ソケットを開く
	this->Socket = ::socket(AF_INET,type,protocol);
	if (this->Socket == INVALID_SOCKET)
	{
		throw XLException( ErrorToMesage( WSAGetLastError() ) );
	}
	SetTimeout(timeout);
} 

void XLSocket::SetTimeout(int timeout)
{
#if _MSC_VER
    int32_t _timeout = timeout * 1000;
    setsockopt(this->Socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&_timeout, sizeof(_timeout));
    setsockopt(this->Socket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&_timeout, sizeof(_timeout));
#else
    struct timeval tv;
    tv.tv_sec  = timeout; 
    tv.tv_usec = 0;         
    setsockopt(this->Socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(this->Socket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
#endif
} 

void XLSocket::CreateTCP(int timeout )
{
	CreateLow(SOCK_STREAM , 0,timeout);
}
void XLSocket::CreateUDP(int timeout  )
{
	CreateLow(SOCK_DGRAM , 0,timeout);
}
void XLSocket::CreateSSL(int timeout  )
{
	CreateLow(SOCK_STREAM , 0,timeout);
	this->UseSSL = true;
}


void XLSocket::Close()
{
	if (this->Socket == INVALID_SOCKET)
	{
		return ;
	}


	if (this->UseSSL)
	{

		if (this->SSLhandle)
		{
//				SSL_shutdown (this->SSLhandle);
				SSL_free (this->SSLhandle);
		}
		if (this->SSLContext)
		{
				SSL_CTX_free (this->SSLContext);
		}

	}

	::closesocket(this->Socket);
	this->Socket = INVALID_SOCKET;

}


int XLSocket::IoctlSocket(long inCmd , unsigned long * ioArgp)
{
	assert(this->Socket != INVALID_SOCKET);

	int ret =  ::ioctlsocket(this->Socket , inCmd , ioArgp);
	if (ret == SOCKET_ERROR )
	{
		throw XLException( ErrorToMesage( WSAGetLastError() ) );
	}
	return ret;
}


int XLSocket::SetSockopt( int level, int optname,  const char * optval,  int optlen )
{
	assert(this->Socket != INVALID_SOCKET);

	return ::setsockopt(this->Socket , level , optname , optval , optlen);
}
int XLSocket::SetSockopt( int level, int optname,  int optval )
{
	assert(this->Socket != INVALID_SOCKET);

	return ::setsockopt(this->Socket , level , optname , (char *)&optval , sizeof(int));
}
int XLSocket::GetSockopt( int level, int optname,  char * optval,  int * optlen )
{
	assert(this->Socket != INVALID_SOCKET);
	
	return ::getsockopt(this->Socket , level , optname , optval , optlen);
}
int XLSocket::GetSockopt( int level, int optname,  int * optval )
{
	assert(this->Socket != INVALID_SOCKET);
	char buffer[20]={0};
	int optlen = sizeof(buffer)-1;
	int r = ::getsockopt(this->Socket , level , optname , buffer , &optlen);

	*optval = atoi(buffer);
	return r;
}

std::map<std::string,std::string> XLSocket::getIPAddressMap() //static 
{
	std::map<std::string,std::string> interfaces;
#if _MSC_VER
	IP_ADAPTER_INFO adapterInfo[256];
	ULONG ulOutBufLen = sizeof(adapterInfo);
	DWORD ret = GetAdaptersInfo(adapterInfo, &ulOutBufLen);
    if (ret != NO_ERROR)
	{
//		DEBUGLOG("IPアドレス一覧が取得できませんでした");
		return interfaces;
	}
	for (const IP_ADAPTER_INFO* pAdapter = adapterInfo; pAdapter; pAdapter = pAdapter->Next) 
	{
		if (strcmp(pAdapter->IpAddressList.IpAddress.String , "0.0.0.0") == 0 )
		{//動いていないものはボツ
//			DEBUGLOG("IPインターフェース " << pAdapter->Description << " は動作していないので無視します");
			continue;
		}
		if (strcmp(pAdapter->GatewayList.IpAddress.String , "0.0.0.0") == 0 )
		{//動いているけど、 gatewayが設定されていないものは無視
//			DEBUGLOG("IPインターフェース " << pAdapter->Description << " の、IPアドレス " << pAdapter->IpAddressList.IpAddress.String << " は、動作しているが gatewayが設定されていないので無視します。");
			continue;
		}

		interfaces[pAdapter->Description] = pAdapter->IpAddressList.IpAddress.String;
//		DEBUGLOG("IPインターフェース " << pAdapter->Description << " の、IPアドレス " << pAdapter->IpAddressList.IpAddress.String << " を発見しました。");
	}
#else
	//http://www.geekpage.jp/programming/linux-network/book/04/4-10.php より
	int socket = ::socket(AF_INET, SOCK_DGRAM, 0);
	if (!socket)
	{
//		DEBUGLOG("IPアドレス一覧が取得できませんでした");
		return interfaces;
	}

	struct ifconf ifc;
	//何個あるか取得する.
	ifc.ifc_len = 0;
	ifc.ifc_ifcu.ifcu_buf = NULL;
	ioctl(socket, SIOCGIFCONF, &ifc);

	//個数分のメモリ確保
	ifc.ifc_ifcu.ifcu_buf = (void *)malloc(ifc.ifc_len);

	//実際の数
	ioctl(socket, SIOCGIFCONF, &ifc);
	unsigned int nifs = ifc.ifc_len / sizeof(struct ifreq);
	struct ifreq * ifr = (struct ifreq *)ifc.ifc_ifcu.ifcu_buf;
	for (unsigned int i=0; i<nifs; i++)
	{
		if (strcmp(ifr[i].ifr_name,"lo")==0)
		{
//			DEBUGLOG("IPインターフェース " << ifr[i].ifr_name << " はローカルループバックなので無視します");
		}
		else
		{
			interfaces[ifr[i].ifr_name] = inet_ntoa(((struct sockaddr_in *)&ifr[i].ifr_addr)->sin_addr);
//			DEBUGLOG("IPインターフェース " << ifr[i].ifr_name << " の、IPアドレス " << interfaces[ifr[i].ifr_name] << " を発見しました。");
		}
	}

	free(ifc.ifc_ifcu.ifcu_buf);
	close(socket);
#endif
	return interfaces;
}


std::string XLSocket::ErrorToMesage(unsigned long inErrorCode) const
{
#if _MSC_VER
	switch(inErrorCode)
	{
	case WSAEINTR: return std::string(" 関数呼び出しが中断された。\nWinSock 1.1 のブロッキング関数呼び出しが、WSACancelBlockingCall() によって中断された。");
	case WSAEBADF: return std::string("無効なファイルハンドル。\nソケット関数に渡されたファイルハンドル（ソケットハンドル）が不正である。（WSAENOTSOCKと同義で用いられる）");
	case WSAEACCES: return std::string("アクセスが拒否された。\nconnect()：ブロードキャストアドレスで接続しようとしたソケットの SO_BROADCASTオプション が有効になっていない。\nsendto()：ブロードキャストアドレスに送信しようとしたソケットの SO_BROADCASTオプションが有効になっていない。\naccept()：受付しようとした接続要求がすでにタイムアウトになっている、もしくは取り消されている。");
	case WSAEFAULT: return std::string("無効なバッファアドレス\n関数に渡されたバッファを指すポインタが無効なアドレス空間を指している。またはバッファのサイズが小さすぎる。");
	case WSAEINVAL: return std::string("無効な引数が渡された。\nlisten()を呼び出さずにaccept()を呼び出した。\nバインド済みのソケットに対してbind()しようとした。\nlisten()で待機中のソケットでconnect()しようとした。\n関数に渡されたパラメータに無効な値が含まれている。");
	case WSAEMFILE: return std::string("使用中のソケットの数が多すぎる。\n同時に利用できるソケット数の最大に達しており、新たにソケットを作成できない。");
//	case WSAEWOULDBLOCK: return std::string("操作はブロッキングされる。\nノンブロッキングモードでソケット関数が呼び出されため、関数は即座にリターンしたが要求された操作は完了していない。");
	case WSAEINPROGRESS: return std::string("すでにブロッキング手続きが実行されている。\nブロッキング関数が実行中であるか、サービスプロバイダがコールバック関数を処理中である。WinSockでは１つのプロセスで同時に複数のブロッキング操作をすることはできないため、前回呼び出した操作が完全に終了するまで次の操作を開始することはできない。");
	case WSAEALREADY: return std::string("要求された操作は既に実行中、または実行済み。\nキャンセルしようとした非同期操作が既にキャンセルされている。\nconnect()を呼び出したが、既に前回の呼び出しによって接続処理中である。");
	case WSAENOTSOCK: return std::string("指定されたソケットが無効である。\nソケットハンドルとして無効な値がソケット関数に渡された。");
	case WSAEMSGSIZE: return std::string("メッセージサイズが大きすぎる。\n送信、または受信しようとしたメッセージが、基盤となるプロトコルのサポートしている最大メッセージサイズを超えている。ioctlsocket()でバッファサイズを増やすことで回避可能なこともある。");
	case WSAEPROTOTYPE: return std::string("ソケットは要求されたプロトコルに適合していない。\nsocket()を呼び出すときに指定したプロトコルタイプがソケットタイプと適合していない。メッセージ型プロトコルでストリームソケットを作成しようとした場合など。");
	case WSAENOPROTOOPT: return std::string("不正なプロトコルオプション。\n指定したソケットオプションまたはioctlsocket()での操作が、そのプロトコルではサポートされてない。\nストリームソケットに SO_BROADCAST を指定しようとした。");
	case WSAEPROTONOSUPPORT: return std::string("プロトコルがサポートされていない。\nサポートされていないプロトコルを指定した、またはプロトコルのサポートしていない操作を実行しようとした。");
	case WSAESOCKTNOSUPPORT: return std::string("指定されたソケットタイプはサポートされていない。");
	case WSAEOPNOTSUPP: return std::string("要求された操作はサポートされていない。\nバンド外データを送信/受信しようとしたソケットでそれがサポートされていない。\n非接続型(UDPなど)ソケットでaccept()を呼び出した。");
	case WSAEPFNOSUPPORT: return std::string("プロトコルファミリがサポートされていない。");
	case WSAEAFNOSUPPORT: return std::string("アドレスファミリがサポートされていない。");
	case WSAEADDRINUSE: return std::string("アドレスは既に使用中である。\nbind()しようとしたアドレスは、既にほかのソケットで使われている。同じローカルアドレスを複数のソケットで使うためには、SO_REUSEADDRソケットオプションをTRUEにする必要がある。");
	case WSAEADDRNOTAVAIL: return std::string("無効なネットワークアドレス。\nソケット関数に渡されたネットワークアドレスに無効な部分がある。");
	case WSAENETDOWN: return std::string("ネットワークがダウンしている。\nネットワークシステムが何らかの障害を起こし、機能停止しているため要求された操作が実行できない。");
	case WSAENETUNREACH: return std::string("指定されたネットワークホストに到達できない。\nWSAEHOSTUNREACHと同義で使われている。");
	case WSAENETRESET: return std::string("ネットワーク接続が破棄された。\nキープアライブを行っている接続で、何らかの障害のためタイムアウトが検出され接続が閉じられた。");
	case WSAECONNABORTED: return std::string("ネットワーク接続が破棄された。");
	case WSAECONNRESET: return std::string("ネットワーク接続が相手によって破棄された。");
	case WSAENOBUFS: return std::string("バッファが不足している。\nメモリ不足のためWinSockが操作に必要なバッファを確保することができない。または、バッファを必要とする関数呼び出しで、小さすぎるバッファが渡された。");
	case WSAEISCONN: return std::string("ソケットは既に接続されている。\n接続中のソケットに対してconnect()、listen()、bind()等の操作を行おうとした。\n接続中のソケットでsendto()を実行しようとした。");
	case WSAENOTCONN: return std::string("ソケットは接続されていない。\n接続されていないソケットでsend()、getpeername()等の接続されたソケットを必要とする操作を実行しようとした。");
	case WSAESHUTDOWN: return std::string("ソケットはシャットダウンされている。\n既にシャットダウンされて送信/受信が停止されているソケットで、送信または受信の操作を実行しようとした。");
	case WSAETOOMANYREFS: return std::string("参照の数が多すぎる。");
	case WSAETIMEDOUT: return std::string("接続要求がタイムアウトした。\nconnect()を呼び出して接続を試みたが、（相手の応答がない等で）処理がタイムアウトになり接続要求がキャンセルされた。");
	case WSAECONNREFUSED: return std::string("接続が拒否された。");
	case WSAELOOP: return std::string("ループ。");
	case WSAENAMETOOLONG: return std::string("名前が長すぎる。");
	case WSAEHOSTDOWN: return std::string("ホストがダウンしている。\nWSAETIMEDOUTと同義。");
	case WSAEHOSTUNREACH: return std::string("ホストへの経路がない。\nネットワークの構造上到達できないホストに対して操作を実行しようとした。またはアドレスが不正である。");
	case WSAENOTEMPTY: return std::string("ディレクトリが空ではない。");
	case WSAEPROCLIM: return std::string("プロセスの数が多すぎる。\nWSAStartup()：既にWinSockが管理できる最大プロセス数に達しており処理が実行できない。");
	case WSAEUSERS: return std::string("ユーザーの数が多すぎる。");
	case WSAEDQUOT: return std::string("ディスククォータ。");
	case WSAESTALE: return std::string("実行しようとした操作は廃止されている。");
	case WSAEREMOTE: return std::string("リモート。");
	case WSASYSNOTREADY: return std::string("ネットワークサブシステムが利用できない。\nWSAStartup()：ネットワークサブシステムが利用できない、または正しくセットアップされていないため機能していない。");
	case WSAVERNOTSUPPORTED: return std::string("Winsock.dllのバージョンが範囲外である。\nWSAStartup()：要求したWinSockバージョンは、現在の実装ではサポートされていない。");
	case WSANOTINITIALISED: return std::string("WinSockシステムが初期化されていない。\nWinSock関数を実行しようとしたが、WSAStartup()が正常に実行されていないため機能しない。");
	case WSAEDISCON: return std::string("シャットダウン処理中。\n接続が相手の処理によって切断されようとしている。");
	case WSAHOST_NOT_FOUND: return std::string("ホストが見つからない。\nDNSなどの名前解決サービスが指定されたホストを見つけられなかった。プロトコルやサービスのクエリー操作においても返される。");
	case WSATRY_AGAIN: return std::string("指定されたホストが見つからない、またはサービスの異常。");
	case WSANO_RECOVERY: return std::string("回復不能なエラーが発生した。\nデータベース検索の場合は、名前解決サービスが使用できないことを意味する。");
	case WSANO_DATA: return std::string("要求されたタイプのデータレコードが見つからない。");
	}
#endif
	return std::string() + "不明のエラーメッセージ " + num2str( inErrorCode);
}

SOCKADDR_IN XLSocket::ToSockAddrIn(const std::string &inHost , int inPort, int sin_family) 
{
	SOCKADDR_IN	sai = {0};
	
	//IP?
	unsigned long connectIP = inet_addr( inHost.c_str() );
	if(connectIP==INADDR_NONE)
	{
		//名前のようなので解決してみる.
		HOSTENT	* hostEnt;
		hostEnt=gethostbyname(inHost.c_str() );
		if(!hostEnt)
		{
			throw XLException("ホスト名を解決できません" + ErrorToMesage(WSAGetLastError() ) );
		}
		sai.sin_addr=*((IN_ADDR*)*hostEnt->h_addr_list);
	}
	else
	{
#if _MSC_VER
		sai.sin_addr.S_un.S_addr = connectIP;
#else
		sai.sin_addr.s_addr = connectIP;
#endif
	}

	sai.sin_port=htons((unsigned short)inPort);
	sai.sin_family=sin_family;

	return sai;
}
SOCKADDR_IN XLSocket::ToSockAddrIn(unsigned long ip , int inPort, int sin_family) 
{
	SOCKADDR_IN	sai = {0};
	sai.sin_family			= sin_family;
	sai.sin_addr.s_addr	=   htonl(ip);
	sai.sin_port			= htons((unsigned short)inPort);

	return sai;
}

SOCKADDR XLSocket::ToSockAddr(const std::string &inHost , int inPort, int sin_family) 
{
	SOCKADDR_IN sai =  ToSockAddrIn(inHost,inPort,sin_family);
	return *((SOCKADDR*)(&sai));
}

SOCKADDR XLSocket::ToSockAddr(unsigned long ip , int inPort, int sin_family) 
{
	SOCKADDR_IN sai =  ToSockAddrIn(ip,inPort,sin_family);
	return *((SOCKADDR*)(&sai));
}

void XLSocket::Connect(const std::string &inHost , int inPort )
{
	assert(this->Socket != INVALID_SOCKET);
	assert(this->Connected == false);

	SOCKADDR_IN	sai = ToSockAddrIn(inHost,inPort);

	//接続を試みる.
	const int ret = connect(Socket,(SOCKADDR *)&sai,sizeof(SOCKADDR_IN));
	if( ret == SOCKET_ERROR )
	{	
		throw XLException("接続に失敗しました" + ErrorToMesage( WSAGetLastError() ) );
	}

	this->Connected = true;

	if (this->UseSSL)
	{

		// Register the error strings for libcrypto & libssl
		//SSL_load_error_strings ();
		// Register the available ciphers and digests
		SSL_library_init ();

		// New context saying we are a client, and using SSL 2 or 3
		this->SSLContext = SSL_CTX_new (SSLv23_client_method ());
		if (this->SSLContext == NULL)
		{
			throw XLException("SSL_CTX_newに失敗しました");
		}

		// Create an SSL struct for the connection
		this->SSLhandle = SSL_new (this->SSLContext);
		if (this->SSLhandle  == NULL)
		{
			throw XLException("SSL_newに失敗しました");
		}

		// Connect the SSL struct to our connection
		if (!SSL_set_fd (this->SSLhandle, this->Socket))
		{
			throw XLException("SSL_set_fdに失敗しました");
		}

		// Initiate SSL handshake
		int r;
		r=SSL_connect (this->SSLhandle);
		if (r != 1)
		{
			throw XLException("SSL_connectに失敗しました");
		}

	}
}


void XLSocket::Bind(unsigned long inBindIP , int inPort)
{
	assert(this->Socket != INVALID_SOCKET);
	assert(this->Connected == false);

	SOCKADDR	sa = ToSockAddr(inBindIP,inPort);
	this->Bind( &sa );
}

void XLSocket::Bind(const std::string& inHost , int inPort)
{
	assert(this->Socket != INVALID_SOCKET);
	assert(this->Connected == false);

	SOCKADDR	sa = ToSockAddr(inHost,inPort);
	this->Bind( &sa );
}

void XLSocket::Bind(const SOCKADDR_IN * inSai)
{
	assert(this->Socket != INVALID_SOCKET);
	assert(this->Connected == false);

	Bind((const SOCKADDR*)inSai);
}

void XLSocket::Bind(const SOCKADDR * inSa)
{
	assert(this->Socket != INVALID_SOCKET);
	assert(this->Connected == false);

	int	Opt = 1;
	this->SetSockopt(SOL_SOCKET, SO_REUSEADDR, (const char*)&Opt , sizeof(int) );

	int ret = ::bind( this->Socket ,inSa , sizeof(SOCKADDR) );
	if (ret < 0)
	{
		throw XLException("bind に失敗しました" + ErrorToMesage( WSAGetLastError() ) );
	}

	this->Connected  = true;
}

void XLSocket::Listen(int inBacklog )
{
	int ret = ::listen( this->Socket , inBacklog );
	if (ret < 0)
	{
		throw XLException("listen に失敗しました" + ErrorToMesage( WSAGetLastError() ) );
	}
}

XLSocket* XLSocket::Accept()
{
	assert(Socket != INVALID_SOCKET);

	SOCKET  newSock = ::accept( this->Socket , (struct sockaddr*) NULL , NULL);
	if (newSock <= 0 )
	{
		throw XLException("accept に失敗しました" + ErrorToMesage( WSAGetLastError() ) );
	}
	if (this->UseSSL)
	{

		SSL_accept(this->SSLhandle);

	}

	//新規に作成して返す
	return new XLSocket(newSock);
}

void XLSocket::Shutdown()
{
	if ( this->Socket == INVALID_SOCKET) return ;
	if ( this->Connected == false)	return ;

	::shutdown( this->Socket , 1 );

	//この時間になってもつないでいる場合は、即切る.
	time_t TimeOutTime = time(NULL) + 10; //10秒待つ.

	//保留中のすべてのデータを受け取る.
	int		ret;
	char	buffer[1024];
	while(1)
	{
		ret = ::recv( this->Socket , buffer , 1024 , 0 );
		
		if ( ret <= 0 || ret == SOCKET_ERROR)	break;

		//タイムアウトチェック
		if ( TimeOutTime < time(NULL) ) 
		{
			break;
		}
	}
	//データをこれ以上受信しないことを相手に伝える.
	::shutdown( this->Socket , 2);

	this->Connected = false;
}

int XLSocket::Send( const std::string&  str )
{
	return this->Send(str.c_str() , str.size() );
}

int XLSocket::Send( const char* inBuffer ,unsigned int inBufferLen  )
{
	unsigned int sended = 0;
	if (this->UseSSL)
	{

		while(1)
		{
			int ret = SSL_write( this->SSLhandle ,(void*) (inBuffer + sended), inBufferLen - sended);
			if (ret < 0)
			{
				return ret;
			}
			sended += ret;
//			printf("SSL_write %d\n",ret);

			if (sended >= inBufferLen) return (int)sended;
		}

	}
	else
	{
		while(1)
		{
			int ret = ::send( this->Socket , inBuffer + sended, inBufferLen - sended, 0);
			if (ret < 0)
			{
				return ret;
			}
			sended += ret;

			if (sended >= inBufferLen) return (int)sended;
		}
	}
	return (int)sended;
}

int XLSocket::Recv( char* outBuffer ,unsigned int inBufferLen )
{
	unsigned int recved = 0;
	if (this->UseSSL)
	{
		while(1)
		{
			int ret = SSL_read( this->SSLhandle ,(void*) (outBuffer + recved), inBufferLen - recved);
//			printf("SSL_read %d\n",ret);
			if (ret < 0)
			{
				break;
			}
			if (ret == 0)
			{
				break;
			}
			recved += ret;

			if (recved >= inBufferLen) return recved;
		}

	}
	else
	{
		while(1)
		{
			int ret = ::recv( this->Socket , outBuffer + recved , inBufferLen - recved, 0);
			if (ret < 0)
			{
				break;
			}
			if (ret == 0)
			{
				break;
			}
			recved += ret;

			if (recved >= inBufferLen) break;
		}
	}
	return recved;
}

int XLSocket::RecvUntil( char* outBuffer ,unsigned int inBufferLen ,std::function<bool(const char*,unsigned int)> func)
{
	unsigned int recved = 0;
	{
		while(1)
		{
			int ret = ::recv( this->Socket , outBuffer + recved , inBufferLen - recved, 0);
			if (ret < 0)
			{
				break;
			}
			if (ret == 0)
			{
				break;
			}
				recved += ret;

			if ( func(outBuffer , recved) )
			{
				break;
			}

			recved += ret;

			if (recved >= inBufferLen) break;
		}
	}
	return recved;
}

int XLSocket::SendTo(const char* inBuffer ,unsigned int inBufferLen , int inFlags ,struct sockaddr_in *senderinfo,int fromlen)
{
	int ret = ::sendto(this->Socket,inBuffer,inBufferLen,inFlags,(struct sockaddr *)senderinfo, fromlen);
	if (ret < 0)
	{
		return ret;
	}
	return ret;
}

int XLSocket::RecvFrom(char* outBuffer ,unsigned int inBufferLen , int inFlags,struct sockaddr_in *senderinfo,int* fromlen )
{
	*fromlen = sizeof(struct sockaddr_in);
	int ret = ::recvfrom(this->Socket,outBuffer,inBufferLen,inFlags,(struct sockaddr *)senderinfo, fromlen );
	if (ret < 0)
	{
		return ret;
	}

	return ret;
}

int XLSocket::getErrorCode() const
{
	return WSAGetLastError();
}
std::string XLSocket::getErrorMessage(int errorcode) const
{
	return ErrorToMesage( errorcode );
}
std::string XLSocket::getErrorMessage() const
{
	return ErrorToMesage( WSAGetLastError() );
}

std::string XLSocket::getMacaddress(const std::string& eth)
{
	std::vector<unsigned char> vec = getMacaddressVec(eth);
	if ( vec.size() != 6 )
	{//macアドレスが、6バイトではないらしい？
		return "";
	}
	
	// 結果を表示 
	char buffer[30];
	sprintf(buffer,"%.2x.%.2x.%.2x.%.2x.%.2x.%.2x\n",
		vec[0],
		vec[1],
		vec[2],
		vec[3],
		vec[4],
		vec[5]
	);

	return buffer;
}

std::vector<unsigned char> XLSocket::getMacaddressVec(const std::string& eth)
{
#if _MSC_VER
	std::vector<unsigned char> ret;
	return ret;
#else
	int fd;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, eth.c_str(), IFNAMSIZ-1);

	ioctl(fd, SIOCGIFHWADDR, &ifr);

	close(fd);

	// 結果を表示 
	
	std::vector<unsigned char> ret;
	ret.push_back((unsigned char)ifr.ifr_hwaddr.sa_data[0]);
	ret.push_back((unsigned char)ifr.ifr_hwaddr.sa_data[1]);
	ret.push_back((unsigned char)ifr.ifr_hwaddr.sa_data[2]);
	ret.push_back((unsigned char)ifr.ifr_hwaddr.sa_data[3]);
	ret.push_back((unsigned char)ifr.ifr_hwaddr.sa_data[4]);
	ret.push_back((unsigned char)ifr.ifr_hwaddr.sa_data[5]);

	return ret;
#endif
}
