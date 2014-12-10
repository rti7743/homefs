﻿// XLSocket.h: XLSocket クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XLHTTPSOCKET_H__137F6EB5_32A1_46CD_9CA7_EC6E9C6A6E6A__INCLUDED_)
#define AFX_XLHTTPSOCKET_H__137F6EB5_32A1_46CD_9CA7_EC6E9C6A6E6A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include <functional>
#include <vector>
#include <map>
#include <list>
#include "XLSocket.h"
#include "XLHttpHeader.h"


class XLHttpSocket  
{
public:
	static std::string Get(const std::string& url);
	static std::string Get(const std::string& url,unsigned int timeout);
	static std::string Get(const std::string& url,const std::map<std::string,std::string> & header);
	static std::string Get(const std::string& url,const std::map<std::string,std::string> & header,unsigned int timeout);
	static std::string Get(const std::string& url,const std::map<std::string,std::string> & header,unsigned int timeout,XLHttpHeader* retHeader);
	static void GetBinary(const std::string& url,const std::map<std::string,std::string> & header,unsigned int timeout,std::vector<char>* retBinary,XLHttpHeader* retHeader);

	static std::string Post(const std::string& url,const char * postBinaryData,unsigned int postBinaryLength);
	static std::string Post(const std::string& url,unsigned int timeout,const char * postBinaryData,unsigned int postBinaryLength);
	static std::string Post(const std::string& url,const std::map<std::string,std::string> & header,const char * postBinaryData,unsigned int postBinaryLength);
	static std::string Post(const std::string& url,const std::map<std::string,std::string> & header,unsigned int timeout,const char * postBinaryData,unsigned int postBinaryLength);
	static std::string Post(const std::string& url,const std::map<std::string,std::string> & header,unsigned int timeout,const std::vector<char>& postBinaryData);
	static std::string Post(const std::string& url,const std::map<std::string,std::string> & header,unsigned int timeout,XLHttpHeader* retHeader,const std::vector<char>& postBinaryData);
	static std::string Post(const std::string& url,const std::map<std::string,std::string> & header,unsigned int timeout,XLHttpHeader* retHeader,const std::string& postString);
	static void PostBinary(const std::string& url,const std::map<std::string,std::string> & header,unsigned int timeout,std::vector<char>* retBinary,XLHttpHeader* retHeader,const char * postBinaryData,unsigned int postBinaryLength);

private:
	static void HTTPRecv(XLSocket * socket ,std::vector<char>* retBinary,XLHttpHeader* retHeader);
};

#endif // !defined(AFX_XLHTTPSOCKET_H__137F6EB5_32A1_46CD_9CA7_EC6E9C6A6E6A__INCLUDED_)
