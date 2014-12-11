/*
*/
#include "common.h"
#include "XLHttpSocket.h"
#include "FHCAPI.h"

bool FHCAPI::setInit(const std::string& mountpoint,const std::string& key)
{
	this->host = mountpoint;
	this->apikey=key;

	if(this->apikey.empty())
	{
		return false;
	}


	return true;
}

const Elec* FHCAPI::findElec(const std::string& name) const
{
	for(auto eit = this->elec.begin();eit!=this->elec.end();eit++)
	{
		const Elec* e = *eit;
		if ( e->name == name )
		{
			return e;
		}
	}
	return NULL;
}


void FHCAPI::parseJsonList(const std::string json,std::vector<std::string>* outVec)const
{
	const char* str = json.c_str();
	str = XLStringUtil::strstr(str,"\"list\":");
	if (!str)
	{
		return ;
	}
	str = XLStringUtil::strstr(str,"[");
	if (!str)
	{
		return ;
	}

	enum state 
	{
		 need_key
		,need_keyend
	};
	state st = need_key;

	const char*p = str+1;
	const char*key = NULL;
	for(;*p;p=XLStringUtil::nextChar(p))
	{
		if(XLStringUtil::isMultiByte(p) || *p == '\\')
		{
			continue;
		}
		if(st==need_key)
		{
			if (*p=='\"')
			{
				key=p+1;
				st=need_keyend;
			}
			else if (*p==']')
			{
				break;
			}
		}
		else if(st==need_keyend)
		{
			if (*p=='\"')
			{
				st=need_key;
				std::string r= std::string(key,0,p-key);
				outVec->push_back(r);
			}
		}
	}
}

void FHCAPI::parseJsonToMap(const std::string json,std::map<std::string,std::string>* outMap) const
{
	const char* str = json.c_str();
	str = XLStringUtil::strstr(str,"{");
	if (!str)
	{
		return ;
	}

	enum state 
	{
		 need_key
		,need_keyend
		,need_value
		,need_valueend
	};
	state st = need_key;

	const char*p = str+1;
	const char*key = NULL;
	const char*keyend = NULL;
	const char*value = NULL;
	for(;*p;p=XLStringUtil::nextChar(p))
	{
		if(XLStringUtil::isMultiByte(p) || *p == '\\')
		{
			continue;
		}
		if(st==need_key)
		{
			if (*p=='\"')
			{
				key=p+1;
				st=need_keyend;
			}
			else if (*p=='}')
			{
				break;
			}
		}
		else if(st==need_keyend)
		{
			if (*p=='\"'&&*(p+1)==':')
			{
				st=need_value;
				keyend=p;
			}
		}
		else if(st==need_value)
		{
			if (*p=='\"')
			{
				value=p+1;
				st=need_valueend;
			}
			else if (*p=='}')
			{
				break;
			}
		}
		else if(st==need_valueend)
		{
			if (*p=='\"')
			{
				st=need_key;

				std::string k= std::string(key,0,keyend-key);
				std::string v= std::string(value,0,p-value);
				(*outMap)[k]=v;
			}
		}
	}
}


//家電の名前一覧を返します
void FHCAPI::api_getlist(std::vector<std::string>* outVec)
{
	try
	{
		std::string url = "http://" + this->host + "/api/elec/getlist?webapi_apikey=" + XLStringUtil::urlencode(_A2U(this->apikey));
		std::string result = XLHttpSocket::Get(url);
		result = _U2A(result);
		std::vector<std::string> vec;

		parseJsonList(result,outVec);
	}
	catch(XLException& e)
	{
		printf("例外:api_getlist %s\n",e.getFullErrorMessage() );
	}
}


//家電のステータスを返します
void FHCAPI::api_getinfo(const std::string& name,std::map<std::string,std::string>* outMap)
{
	try
	{
		std::string url = "http://" + this->host + "/api/elec/getinfo?webapi_apikey=" + XLStringUtil::urlencode(_A2U(this->apikey)) + "&elec=" + XLStringUtil::urlencode(_A2U(name));
		std::string result = XLHttpSocket::Get(url);
		result = _U2A(result);
		std::vector<std::string> vec;

		parseJsonToMap(result,outMap);
	}
	catch(XLException& e)
	{
		printf("例外:api_getinfo %s\n",e.getFullErrorMessage() );
	}
}

//家電の状態一覧を返します
void FHCAPI::api_getactionlist(const std::string& name,std::vector<std::string>* outVec)
{
	try
	{
		std::string url = "http://" + this->host + "/api/elec/getactionlist?webapi_apikey=" + XLStringUtil::urlencode(_A2U(this->apikey)) + "&elec=" + XLStringUtil::urlencode(_A2U(name));
		std::string result = XLHttpSocket::Get(url);
		result = _U2A(result);

		parseJsonList(result,outVec);
	}
	catch(XLException& e)
	{
		printf("例外:api_getactionlist %s\n",e.getFullErrorMessage() );
	}
}

//家電をその状態にする.
bool FHCAPI::fire(const std::string& elecname,const std::string& actionname) 
{
	try
	{
		std::string url = "http://" + this->host + "/api/elec/action?webapi_apikey=" + XLStringUtil::urlencode(this->apikey) + "&elec=" + XLStringUtil::urlencode(_A2U(elecname)) + "&action=" + XLStringUtil::urlencode(_A2U(actionname));
		std::string result = XLHttpSocket::Get(url);
		result = _U2A(result);

		std::map<std::string,std::string> map;
		parseJsonToMap(result,&map);

		return true;
	}
	catch(XLException& e)
	{
		printf("例外:fire %s\n",e.getFullErrorMessage() );
		return false;
	}
	
}

void FHCAPI::freeElec()
{
	for(auto eit = this->elec.begin();eit!=this->elec.end();eit++)
	{
		Elec* e = *eit;
		for(auto ait = e->action.begin();ait!=e->action.end();ait++)
		{
			delete *ait;
		}
		delete *eit;
	}
	this->elec.clear();
}


bool FHCAPI::update()
{
	freeElec();

	std::vector<std::string> elecVec;
	api_getlist(&elecVec);
	if(elecVec.empty())
	{
		return false;
	}

	for(auto eit = elecVec.begin();eit!=elecVec.end();eit++)
	{
		Elec* e = new Elec;
		e->name = *eit;

		std::vector<std::string> actionVec;
		api_getactionlist(*eit,&actionVec);
		for(auto ait = actionVec.begin();ait!=actionVec.end();ait++)
		{
			Action* a = new Action;
			a->name = *ait;
			e->action.push_back(a);
		}

		std::map<std::string,std::string> elecInfoMap;
		api_getinfo(*eit,&elecInfoMap);
		
		e->status = elecInfoMap["status"];
		this->elec.push_back(e);
	}

	return true;
}

//この状態名があるかどうか調べようか.
bool FHCAPI::checkStatus(const std::string& elecname,const std::string& actionname) const
{
	const Elec* e = findElec(elecname);
	if (!e)
	{
		return false;
	}

	for(auto ait = e->action.begin();ait!=e->action.end();ait++)
	{
		if( (*ait)->name == actionname)
		{
			return true;
		}
	}

	return false;
}


