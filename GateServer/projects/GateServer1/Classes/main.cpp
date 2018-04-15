// testlibeventSrv.cpp : 定义控制台应用程序的入口点。
//

#ifndef __MAIN_H__
#define __MAIN_H__
#include "stdafx.h"
#include "HttpLogic.h"
#include "thread"
#include "LibEvent.h"
#include "Poco/Util/Timer.h"
#include "LoginInfo.h"
#include "HallInfo.h"
#include "ClientSocket.h"
#include "LoginInfo.h"
#include "StatTimer.h"
#include "RedisGet.h"
#include "ConfigData.h"
#include "Common.h"
#include "HttpPay.h"
#include "HttpWXLogin.h"
#include "openapi_client.h"
#include "HttpAliPay.h"
JsonMap getPrecreateContent() {

	JsonMap contentMap;
	contentMap.insert(JsonMap::value_type(JsonType("out_trade_no"), JsonType("20160606121212")));
	contentMap.insert(JsonMap::value_type(JsonType("total_amount"), JsonType(0.01)));
	contentMap.insert(JsonMap::value_type(JsonType("subject"), JsonType(XXIconv::GBK2UTF("好东西").c_str())));

	return contentMap;
}

int _tmain(int argc, _TCHAR* argv[])
{

 	//OpenapiClient oc(ALIAPPID, ALIPRIKEY,ALIPAYURL,CHARSET,ALIPUBKEY);
 	//JsonMap mm= oc.invoke("alipay.trade.precreate", getPrecreateContent());
	//HttpAliPay::getIns()->requestOrder("100001","1",1,"test 1","");

	StatTimer::getIns();
	CSVDataInfo::getIns();
	LoginInfo::getIns();
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 1), &wsaData);
	redis::getIns()->initial("47.104.165.65", 6379, "3.1415926qp");
	RedisGet::getIns()->init();
	
	//ConfigData::getIns();
	
	LoginInfo::getIns()->openCheckUpdate(true);
	HallInfo::getIns();

	HttpLogic::getIns()->requestManagerData();
	HttpLogic::getIns()->requestGateData();
	HttpWXLogin::getIns();
	HttpPay::getIns()/*->test()*/;

	getchar();
	return 0;
}

#endif