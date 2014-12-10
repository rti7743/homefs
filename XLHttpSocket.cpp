#include "common.h"
#include "XLException.h"
#include "XLHttpSocket.h"
#include "XLUrlParser.h"
//#include "XLGZip.h"

std::string XLHttpSocket::Get(const std::string& url)
{
	const std::map<std::string,std::string> header;
	return Get(url,header,60);
}

std::string XLHttpSocket::Get(const std::string& url,unsigned int timeout)
{
	const std::map<std::string,std::string> header;
	return Get(url,header,timeout);
}

std::string XLHttpSocket::Get(const std::string& url,const std::map<std::string,std::string> & header)
{
	return Get(url,header,60);
}

std::string XLHttpSocket::Get(const std::string& url,const std::map<std::string,std::string> & header,unsigned int timeout)
{
	std::vector<char> retBinary;
	XLHttpHeader retHeader;
	GetBinary(url,header,timeout,&retBinary,&retHeader);

	if (retBinary.empty()) return "";
	return std::string( &retBinary[0] , 0 , retBinary.size() );
}

std::string XLHttpSocket::Get(const std::string& url,const std::map<std::string,std::string> & header,unsigned int timeout,XLHttpHeader * retheader)
{
	std::vector<char> retBinary;
	GetBinary(url,header,timeout,&retBinary,retheader);

	if (retBinary.empty()) return "";
	return std::string( &retBinary[0] , 0 , retBinary.size() );
}



void XLHttpSocket::GetBinary(const std::string& url,const std::map<std::string,std::string> & header,unsigned int timeout,std::vector<char>* retBinary,	XLHttpHeader* retHeader)
{
	retBinary->clear();

	//URLパース
	XLUrlParser urlparse;
	urlparse.Parse(url);

	XLSocket socket;
	if ((urlparse.getProtocol() == "https"))
	{
		socket.CreateSSL(timeout);
	}
	else
	{
		socket.CreateTCP(timeout);
	}
	socket.Connect( urlparse.getHost() , urlparse.getPort() );

	//送信ヘッダーの準備
	std::string sendstring;
	sendstring = "GET " + urlparse.getPath() + " HTTP/1.0\r\n";
	for(auto headerit = header.begin() ; header.end() != headerit ; ++headerit)
	{
		sendstring += XLStringUtil::HeaderUpperCamel(headerit->first) + ": " + headerit->second + "\r\n";
	}
	if ( header.find("host") == header.end())
	{
		sendstring += "Host: " + urlparse.getHost() + "\r\n";
	}
	if ( header.find("connection") == header.end())
	{
		sendstring += "Connection: close\r\n";
	}
	if ( urlparse.IsAuthParam() && header.find("authorization") == header.end())
	{
		sendstring += "Authorization: Basic " + XLStringUtil::base64encode(urlparse.getUserName() + ":" + urlparse.getPassword()) + "\r\n";
	}
	sendstring += "\r\n";

	int ret = socket.Send(sendstring.c_str() ,sendstring.size() );
	if (ret < 0)
	{
		int err = socket.getErrorCode();
		throw XLException("ヘッダー送信中にエラー " + num2str(err) );
	}
	//結果の受信
	HTTPRecv( &socket , retBinary,retHeader);
}


std::string XLHttpSocket::Post(const std::string& url,const char * postBinaryData,unsigned int postBinaryLength)
{
	const std::map<std::string,std::string> header;
	return Post(url,header,60,postBinaryData,postBinaryLength);
}

std::string XLHttpSocket::Post(const std::string& url,unsigned int timeout,const char * postBinaryData,unsigned int postBinaryLength)
{
	const std::map<std::string,std::string> header;
	return Post(url,header,timeout,postBinaryData,postBinaryLength);
}

std::string XLHttpSocket::Post(const std::string& url,const std::map<std::string,std::string> & header,const char * postBinaryData,unsigned int postBinaryLength)
{
	return Post(url,header,60,postBinaryData,postBinaryLength);
}

std::string XLHttpSocket::Post(const std::string& url,const std::map<std::string,std::string> & header,unsigned int timeout,const char * postBinaryData,unsigned int postBinaryLength)
{
	std::vector<char> retBinary;
	XLHttpHeader retHeader;
	PostBinary(url,header,timeout,&retBinary,&retHeader,postBinaryData,postBinaryLength);

	if (retBinary.empty()) return "";
	return std::string( &retBinary[0] , 0 , retBinary.size() );
}

std::string XLHttpSocket::Post(const std::string& url,const std::map<std::string,std::string> & header,unsigned int timeout,const std::vector<char>& postBinaryData)
{
	std::vector<char> retBinary;
	XLHttpHeader retHeader;
	PostBinary(url,header,timeout,&retBinary,&retHeader,&postBinaryData[0],postBinaryData.size() );

	if (retBinary.empty()) return "";
	return std::string( &retBinary[0] , 0 , retBinary.size() );
}

std::string XLHttpSocket::Post(const std::string& url,const std::map<std::string,std::string> & header,unsigned int timeout,XLHttpHeader* retHeader,const std::vector<char>& postBinaryData)
{
	std::vector<char> retBinary;
	PostBinary(url,header,timeout,&retBinary,retHeader,&postBinaryData[0],postBinaryData.size() );

	if (retBinary.empty()) return "";
	return std::string( &retBinary[0] , 0 , retBinary.size() );
}

std::string XLHttpSocket::Post(const std::string& url,const std::map<std::string,std::string> & header,unsigned int timeout,XLHttpHeader* retHeader,const std::string& postString)
{
	std::vector<char> retBinary;
	PostBinary(url,header,timeout,&retBinary,retHeader,postString.c_str(),postString.size() );

	if (retBinary.empty()) return "";
	return std::string( &retBinary[0] , 0 , retBinary.size() );
}



void XLHttpSocket::PostBinary(const std::string& url,const std::map<std::string,std::string> & header,unsigned int timeout,std::vector<char>* retBinary,XLHttpHeader* retHeader,const char * postBinaryData,unsigned int postBinaryLength)
{
	retBinary->clear();

	//URLパース
	XLUrlParser urlparse;
	urlparse.Parse(url);

	XLSocket socket;
	if ( (urlparse.getProtocol() == "https") )
	{
		socket.CreateSSL(timeout);
	}
	else
	{
		socket.CreateTCP(timeout);
	}
	socket.Connect( urlparse.getHost() , urlparse.getPort() );

	//送信ヘッダーの準備
	std::string sendstring;
	sendstring = "POST " + urlparse.getPath() + " HTTP/1.0\r\n";
	for(auto headerit = header.begin() ; header.end() != headerit ; ++headerit)
	{
		sendstring += XLStringUtil::HeaderUpperCamel(headerit->first) + ": " + headerit->second + "\r\n";
	}
	if ( header.find("host") == header.end())
	{
		sendstring += "Host: " + urlparse.getHost() + "\r\n";
	}
	if ( header.find("connection") == header.end())
	{
		sendstring += "Connection: close\r\n";
	}
	if ( header.find("content-length") == header.end())
	{
		sendstring += "Content-Length: " + num2str(postBinaryLength) + "\r\n";
	}
	if ( urlparse.IsAuthParam() && header.find("authorization") == header.end())
	{
		sendstring += "Authorization: " + XLStringUtil::base64encode(urlparse.getUserName() + ":" + urlparse.getPassword()) + "\r\n";
	}
	sendstring += "\r\n";

	int ret;
	ret = socket.Send(sendstring.c_str() ,sendstring.size()   );
	if (ret < 0)
	{
		int err = socket.getErrorCode();
		throw XLException("ヘッダー送信中にエラー " + num2str(err) );
	}
	ret = socket.Send(postBinaryData     ,postBinaryLength   );
	if (ret < 0)
	{
		int err = socket.getErrorCode();
		throw XLException("POSTコンテンツ送信中にエラーー " + num2str(err) );
	}
	//結果の受信
	HTTPRecv( &socket , retBinary,retHeader);
}



void XLHttpSocket::HTTPRecv(XLSocket * socket , std::vector<char>* retBinary,XLHttpHeader* retHeader)
{
	std::vector<char> buffermalloc(65535);
	const unsigned int buffersize = buffermalloc.size() - 1;
	char * buffer = &buffermalloc[0];

	//まずヘッダーのパース
	int size = socket->Recv(buffer,buffersize);
	if (size <= 0)
	{
		int err = socket->getErrorCode();
		throw XLException("受信中にエラー " + num2str(err) );
	}
	buffer[size] = '\0';
	if (! retHeader->Parse(buffer) )
	{
		throw XLException("ヘッダーパース中にエラーが発生しました");
	}
	//ヘッダーとボディを分離する
	retBinary->insert(retBinary->end() ,buffer + retHeader->getHeaderSize() ,buffer + size);

	//まだボディが続く場合は読み込む
	while(1)
	{
		size = socket->Recv(buffer,buffersize);
		if (size == 0)
		{
			break;
		}
		else if (size <= 0)
		{
			int err = socket->getErrorCode();
			throw XLException("受信中にエラーが発生しました err:"+ num2str(err) );
		}
		
		retBinary->insert(retBinary->end() ,buffer,buffer +size );
	}
/*
とりあえず今回は使わない
	if ( retHeader->getAt("content-encoding").find("gzip") != std::string::npos )
	{//gzip圧縮されているらしいよ!
		XLGZip zip;
		
		if ( zip.gunzip(&((*retBinary)[0]),retBinary->size() ) )
		{//gzip解凍できたから、バッファを入れ替える
			retBinary->clear();
			retBinary->insert(retBinary->end() ,zip.getData() , zip.getData() + zip.getSize() );
		}
	}
*/
}

