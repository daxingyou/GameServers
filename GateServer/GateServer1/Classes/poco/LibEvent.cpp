﻿#include "LibEvent.h"
#include "LogicServerInfo.h"
#include "LoginInfo.h"
#include "HttpLogic.h"
#include "Common.h"
#include "RoomInfo.h"

LibEvent *LibEvent::m_ins = NULL;
LibEvent::LibEvent()
{
	LogicServerInfo::getIns();
	LoginInfo::getIns();
	ZeroMemory(&m_Server, sizeof(m_Server));
	WSADATA WSAData;
	WSAStartup(0x0201, &WSAData);
		
}

LibEvent::~LibEvent()
{
	WSACleanup();
}

LibEvent *LibEvent::getIns(){
	if (!m_ins){
		m_ins = new LibEvent();
	}
	return m_ins;
}

bool LibEvent::StartServer(int port, short workernum, unsigned int connnum, int read_timeout, int write_timeout)
{

	m_Server.bStart = false;
	m_Server.nCurrentWorker = 0;
	m_Server.nPort = port;
	m_Server.workernum = workernum;
	m_Server.connnum = connnum;
	m_Server.read_timeout = read_timeout;
	m_Server.write_timeout = write_timeout;
	evthread_use_windows_threads();
	m_Server.pBase = event_base_new();
	if (m_Server.pBase == NULL)
	{
		return false;
	}
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(m_Server.nPort);
	m_Server.pListener = evconnlistener_new_bind(m_Server.pBase, DoAccept, (void*)&m_Server, LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1, (struct sockaddr*)&sin, sizeof(sin));
	if (m_Server.pListener == NULL)
	{
		return false;
	}

	m_Server.pWorker = new Worker[workernum];
	for (int i = 0; i < workernum; i++)
	{
		m_Server.pWorker[i].pWokerbase = event_base_new();
		if (m_Server.pWorker[i].pWokerbase == NULL)
		{
			delete[]m_Server.pWorker;
			return false;
		}
		//初始化连接对象
		{
			m_Server.pWorker[i].pListConn = new ConnList();
			if (m_Server.pWorker[i].pListConn == NULL)
			{
				return false;
			}
			m_Server.pWorker[i].pListConn->plistConn = new Conn[m_Server.connnum + 1];
			m_Server.pWorker[i].pListConn->head = &m_Server.pWorker[i].pListConn->plistConn[0];
			m_Server.pWorker[i].pListConn->tail = &m_Server.pWorker[i].pListConn->plistConn[m_Server.connnum];
			for (int j = 0; j < m_Server.connnum; j++) {
				m_Server.pWorker[i].pListConn->plistConn[j].index = j;
				m_Server.pWorker[i].pListConn->plistConn[j].next = &m_Server.pWorker[i].pListConn->plistConn[j + 1];
			}
			m_Server.pWorker[i].pListConn->plistConn[m_Server.connnum].index = m_Server.connnum;
			m_Server.pWorker[i].pListConn->plistConn[m_Server.connnum].next = NULL;
			//设置当前事件
			Conn *p = m_Server.pWorker[i].pListConn->head;
			while (p != NULL)
			{
				p->bufev = bufferevent_socket_new(m_Server.pWorker[i].pWokerbase, -1, BEV_OPT_CLOSE_ON_FREE);
				if (p->bufev == NULL)
				{
					return false;
				}
				bufferevent_setcb(p->bufev, DoRead, NULL, DoError, p);
				bufferevent_setwatermark(p->bufev, EV_READ, 0, emMaxBuffLen);
				bufferevent_enable(p->bufev, EV_READ | EV_WRITE);
				struct timeval delayWriteTimeout;
				delayWriteTimeout.tv_sec = m_Server.write_timeout;
				delayWriteTimeout.tv_usec = 0;
				struct timeval delayReadTimeout;
				delayReadTimeout.tv_sec = m_Server.read_timeout;
				delayReadTimeout.tv_usec = 0;
				//bufferevent_set_timeouts(p->bufev, &delayReadTimeout, &delayWriteTimeout);
				p->owner = &m_Server.pWorker[i];
				p = p->next;
			}
		}
		m_Server.pWorker[i].hThread = CreateThread(NULL, 0, ThreadWorkers, &m_Server.pWorker[i], 0, NULL);
	}
	m_Server.hThread = CreateThread(NULL, 0, ThreadServer, &m_Server, 0, NULL);
	if (m_Server.hThread == NULL)
	{
		return false;
	}
	m_Server.bStart = true;

	CreateThread(NULL, 0, ThreadHttp, NULL, 0, NULL);
		
	return true;
}

void LibEvent::StopServer()
{
	if (m_Server.bStart)
	{
		struct timeval delay = { 2, 0 };
		event_base_loopexit(m_Server.pBase, &delay);
		WaitForSingleObject(m_Server.hThread, INFINITE);
		if (m_Server.pWorker)
		{
			for (int i = 0; i < m_Server.workernum; i++)
			{
				event_base_loopexit(m_Server.pWorker[i].pWokerbase, &delay);
				WaitForSingleObject(m_Server.pWorker[i].hThread, INFINITE);
			}
			for (int i = 0; i < m_Server.workernum; i++)
			{
				if (m_Server.pWorker[i].pListConn)
				{
					delete[]m_Server.pWorker[i].pListConn->plistConn;
					delete m_Server.pWorker[i].pListConn;
					m_Server.pWorker[i].pListConn = NULL;
				}
				event_base_free(m_Server.pWorker[i].pWokerbase);
			}
			delete[]m_Server.pWorker;
			m_Server.pWorker = NULL;
		}
		evconnlistener_free(m_Server.pListener);
		event_base_free(m_Server.pBase);
	}
	m_Server.bStart = false;
}

void LibEvent::DoRead(struct bufferevent *bev, void *ctx)
{
	char headchar[10];
	Conn *c = (Conn *)ctx;
	while (1) {
		size_t len = bufferevent_read(bev, headchar, 10);
		int llen = len;
		if (len > 0){
			LibEvent *pLibEvent = LibEvent::getIns();
			ClientData *data = pLibEvent->getClientData(c->fd);
			if (data){
				data->m_lasttime = Common::getTime();
			}
			Head *testhead = (Head*)headchar;
			string serverdest = pLibEvent->getReq(testhead);
			int cmd = pLibEvent->getCMD(testhead);
			int bodylen = pLibEvent->getBodyLen(testhead);
			int stamp = pLibEvent->getStamp(testhead);
			char *buffer = new char[bodylen];
			llen = bufferevent_read(bev, buffer, bodylen);
			CLog::log("libevent threadHandler:cmd[0x%04X],len[%d],servercode[%s]\n", cmd, bodylen, serverdest.c_str());
			c->m_recvstamp = (c->m_recvstamp + 1) % MAXSTAMP;
			CLog::log("len[%d]==bodylen[%d]  server stamp[%d]==stamp[%d]\n", llen, bodylen, stamp, c->m_recvstamp);
			if (llen == bodylen&&c->m_recvstamp == stamp){
				char* out = new char[llen + 1];
				HttpLogic::getIns()->aes_decrypt(buffer, llen, out);
				delete buffer;
				
				ccEvent *cce = new ccEvent(cmd, out, llen, c->fd, GAME_TYPE);
				EventDispatcher::getIns()->disEventDispatcher(cce);
			}
			else{
				CLog::log("LibEvent数据不合法！！！！！！！！\n");
				LibEvent::getIns()->CloseConn(c);
			}
		}
		else{
			break;
		}
	}
}

void LibEvent::SendData(int cmd, const google::protobuf::Message *msg, evutil_socket_t fd){
	CLog::log("[0x%04X]%s\n",cmd,msg->DebugString().c_str());
	ClientData *pdata = getClientData(fd);
	if (pdata&&pdata->_conn){
		pdata->_conn->m_sendstamp = (pdata->_conn->m_sendstamp + 1) % MAXSTAMP;
		CLog::log("tamp:%d\n", pdata->_conn->m_sendstamp);
		int len = msg->ByteSize();
		char *buffer = new char[HEADLEN + len];
		memset(buffer, 0, HEADLEN + len);

		//服务器编号
		memcpy(buffer, HttpLogic::SERVER_CODE.c_str(), 3);
		
		//消息序列号
		buffer[3] = pdata->_conn->m_sendstamp;
		//bodylen
		char * clen = (char *)&len;
		for (int i = 0; i < 2; i++){
			buffer[4 + i] = *(clen + i);
		}
		//cmd
		char *ccmd = (char *)&cmd;
		for (int i = 0; i < 4; i++){
			buffer[6 + i] = *(ccmd + i);
		}

		char* sm=new char[len];
		msg->SerializePartialToArray(sm,len);
		char *out = new char[len+1];
		HttpLogic::getIns()->aes_encrypt(sm, len, out);
		delete sm;
		for (int i = HEADLEN; i < HEADLEN + len; i++){
			buffer[i] = out[i - HEADLEN];
		}
		
		bufferevent_write(pdata->_conn->bufev, buffer, len + HEADLEN);
		delete out;
		delete buffer;
	}
}

void LibEvent::CloseConn(Conn *pConn, int nFunID)
{
	int fd = pConn->fd;
	if (fd > 0){
		eraseClientData(fd);
	}
}

void LibEvent::resetConn(Conn *pConn){
	if (pConn&&pConn->fd>0){
		bufferevent_disable(pConn->bufev, EV_READ | EV_WRITE);
		evutil_closesocket(pConn->fd);
		pConn->m_sendstamp = 0;
		pConn->m_recvstamp = 0;
		pConn->owner->PutFreeConn(pConn);
		pConn->fd = 0;
	}
}

void LibEvent::CloseConn(Conn *pConn)
{
	CloseConn(pConn, emFunClosed);
		
}

void LibEvent::DoError(struct bufferevent *bev, short error, void *ctx)
{
	Conn *c = (Conn*)ctx;
	emFunID id = emFunNull;
	if (error&EVBUFFER_TIMEOUT)
	{
		id = emFunTimeout;
	}
	else if (error&EVBUFFER_EOF)
	{
		id = emFunClosed;
	}
	else if (error&EVBUFFER_ERROR)
	{
		id = emFunError;
	}
	LibEvent::getIns()->CloseConn(c, id);
}

void LibEvent::DoAccept(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sa, int socklen, void *user_data)
{
	//此处为监听线程的event.不做处理.
	Server *pServer = (Server *)user_data;
	//主线程处做任务分发.
	int nCurrent = pServer->nCurrentWorker++%pServer->workernum;
	//当前线程所在ID号
	Worker &pWorker = pServer->pWorker[nCurrent];
	//通知线程开始读取数据,用于分配哪一个线程来处理此处的event事件
	Conn *pConn = pWorker.GetFreeConn();
	if (pConn == NULL)
	{
		return;
	}
	struct sockaddr_in * in = (struct sockaddr_in *)sa;
	string ip = inet_ntoa(in->sin_addr);
	CLog::log("accept IP:%s\n",ip.c_str());
	pConn->fd = fd;

	evutil_make_socket_nonblocking(pConn->fd);
	bufferevent_setfd(pConn->bufev, pConn->fd);
	//记录连接的信息 fd关键
	ClientData *data = new ClientData();
	data->_fd = fd;
	data->_ip = ip;
	data->_conn = pConn;
	data->m_lasttime = Common::getTime();
	LibEvent *pLibEvent = LibEvent::getIns();
	pLibEvent->inserClientData(fd, data);
	bufferevent_enable(pConn->bufev, EV_READ | EV_WRITE);
}

DWORD WINAPI LibEvent::ThreadAI(LPVOID lPVOID){
		
	return GetCurrentThreadId();
}

DWORD WINAPI LibEvent::ThreadHttp(LPVOID lPVOID){
	HttpEvent::getIns()->init();

	return GetCurrentThreadId();
}

DWORD WINAPI LibEvent::ThreadServer(LPVOID lPVOID)
{
	Server * pServer = reinterpret_cast<Server *>(lPVOID);
	if (pServer == NULL)
	{
		return -1;
	}
	event_base_dispatch(pServer->pBase);
	return GetCurrentThreadId();
}

DWORD WINAPI LibEvent::ThreadWorkers(LPVOID lPVOID)
{
	Worker *pWorker = reinterpret_cast<Worker *>(lPVOID);
	if (pWorker == NULL)
	{
		return -1;
	}
	event_base_dispatch(pWorker->pWokerbase);
	return GetCurrentThreadId();
}

string LibEvent::getReq(Head *h){
	char buff[10];
	memset(buff,0,10);
	memcpy(buff,h->_req,3);
	return buff;
}

int LibEvent::getCMD(Head *h){
	int cmd = 0;
	memcpy(&cmd, h->_cmd, 4);
	return cmd;
}

int LibEvent::getBodyLen(Head *h){
	int len = 0;
	memcpy(&len, h->_bodylen, 2);
	return len;
}

int LibEvent::getStamp(Head *h){
	int stamp = h->_stamp;
	return stamp;
}

void LibEvent::inserClientData(int fd, ClientData *data){
	if (m_ClientDatas.find(fd) == m_ClientDatas.end()){
		m_ClientDatas.insert(make_pair(fd, data));
	}
}

ClientData * LibEvent::getClientData(int fd){
	if (m_ClientDatas.find(fd) != m_ClientDatas.end()){
		return m_ClientDatas.at(fd);
	}
	return NULL;
}

int LibEvent::getFd(string uid){
	ClientData *data = getClientDataByUID(uid);
	if (data){
		return data->_fd;
	}
	return -1;
}

ClientData * LibEvent::getClientData(string sessionid){
	map<int , ClientData *>::iterator itr = m_ClientDatas.begin();
	for (itr;itr!=m_ClientDatas.end(); itr++){
		ClientData *data = itr->second;
		if (data&&data->_sessionID.compare(sessionid) == 0){
			return data;
		}
	}
	return NULL;
}

void LibEvent::eraseClientData(int fd){
	if (m_ClientDatas.find(fd) != m_ClientDatas.end()){
		ClientData *data = m_ClientDatas.at(fd);
		m_ClientDatas.erase(m_ClientDatas.find(fd));
		if (data){
			CLog::log("close ip:%s\n",data->_ip.c_str());
			if (data->_conn){
				RedisPut::getIns()->PushUserLoginTime(data->_uid);
				LoginInfo::getIns()->inserUserBase(data->_uid,RedisGet::getIns()->getUserBase(data->_uid), false);
				SendLeave(data->_uid);
				resetConn(data->_conn);
			}
			delete data;
		}
	}
}

void LibEvent::eraseClientData(string sesionid){
	map<int , ClientData *>::iterator itr = m_ClientDatas.begin();
	for (itr; itr != m_ClientDatas.end(); itr++){
		ClientData *data = itr->second;
		if (data&&data->_sessionID.compare(sesionid) == 0){
			m_ClientDatas.erase(itr);
			if (data){
				CLog::log("close ip:%s\n",data->_ip.c_str());
				if (data->_conn){
					RedisPut::getIns()->PushUserLoginTime(data->_uid);
					LoginInfo::getIns()->inserUserBase(data->_uid, RedisGet::getIns()->getUserBase(data->_uid), false);
					SendLeave(data->_uid);
					CloseConn(data->_conn, emFunClosed);
					resetConn(data->_conn);
				}
				delete data;
			}
			return;
		}
	}
}

void LibEvent::SendLeave(string uid){
	CLeave line;
	line.set_cmd(line.cmd());
	line.set_uid(uid);
	line.set_zhudong(false);
	int sz = line.ByteSize();
	char *buffer = new char[sz];
	line.SerializePartialToArray(buffer, sz);
	ccEvent *ev = new ccEvent(line.cmd(), buffer, sz, -1, GAME_TYPE);
	RoomInfo::getIns()->HandCLeave(ev);
	delete ev;
	ev = NULL;
}

ClientData *LibEvent::getClientDataByUID(string uid){
	map<int, ClientData *>::iterator itr = m_ClientDatas.begin();
	for (itr; itr != m_ClientDatas.end(); itr++){
		ClientData *data = itr->second;
		if (data&&data->_uid.compare(uid) == 0){
			return data;
		}
	}
	return NULL;
}

bool LibEvent::isHave(string uid){
	ClientData *data = getClientDataByUID(uid);
	return data ? true : false;
}

string LibEvent::getUID(int fd){
	ClientData *data = getClientData(fd);
	if (data){
		return data->_uid;
	}
	return "";
}
