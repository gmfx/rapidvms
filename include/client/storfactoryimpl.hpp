//------------------------------------------------------------------------------
// File: storfactory impl.hpp
//
// Desc: Camera storfactory - Manage IP Camera.
//
// Copyright (c) 2014-2018 opencvr.com. All rights reserved.
//------------------------------------------------------------------------------

#ifndef __VSC_STOR_FACTORY_IMPL_H_
#define __VSC_STOR_FACTORY_IMPL_H_



inline StorFactory::StorFactory(ClientConfDB &pConf)
:m_pConf(pConf)
{

}

inline StorFactory::~StorFactory()
{

}

inline BOOL StorFactory::Init()
{

	/* Loop add the stor */
	VidStorList storList;
	m_pConf.GetStorListConf(storList);
	int storSize = storList.cvidstor_size();

	for (s32 i = 0; i < storList.cvidstor_size(); i ++)
	{
		VidStor pStor = storList.cvidstor(i);
		InitAddStor(pStor);
	}

	return TRUE;
}

inline bool StorFactory::InitAddStor(VidStor &pStor)
{
	StorFactoryNotifyInterface &pNotify = *this;
	m_StorClientMap[pStor.strid()] = new StorClient(pStor, pNotify);

       m_StorClientOnlineMap[pStor.strid()] = false;

    	return true;
}

inline bool StorFactory::AddStor(VidStor & pParam)
{
	XGuard guard(m_cMutex);
	StorFactoryChangeData change;
	
	if (m_pConf.FindStor(pParam.strid()))
	{

		m_pConf.DeleteStor(pParam.strid());

		/* Call Change */
		change.cId.set_strstorid(pParam.strid());
		change.type = STOR_FACTORY_STOR_DEL;
		guard.Release();
		CallChange(change);
		guard.Acquire();
		
		delete m_StorClientMap[pParam.strid()];
		m_StorClientMap[pParam.strid()] = NULL;
		m_StorClientMap.erase(pParam.strid());
		m_StorClientOnlineMap.erase(pParam.strid());
	}

	/* Add */
	m_pConf.AddStor(pParam);
	InitAddStor(pParam);

	/* Call Change */
	change.cId.set_strstorid(pParam.strid());
	change.type = STOR_FACTORY_STOR_ADD;
	guard.Release();
	CallChange(change);
	guard.Acquire();

	return true;
}
inline bool StorFactory::DeleteStor(astring strId)
{
	XGuard guard(m_cMutex);
	StorFactoryChangeData change;

	if (m_pConf.FindStor(strId))
	{
		m_pConf.DeleteStor(strId);
		/* Call Change */
		change.cId.set_strstorid(strId);
		change.type = STOR_FACTORY_STOR_DEL;
		guard.Release();
		CallChange(change);
		guard.Acquire();
		
		//delete m_StorClientMap[strId];
		/* the StorClient will be delete itself */
		m_StorClientMap[strId] = NULL;
		m_StorClientMap.erase(strId);
		m_StorClientOnlineMap.erase(strId);
		
	}

	return true;
}

inline bool StorFactory::CallChange(StorFactoryChangeData data)
{
#if 0
	//XGuard guard(m_cMutex);
	StorChangeNofityMap::iterator it = m_Change.begin(); 
	for(; it!=m_Change.end(); ++it)
	{
		if ((*it).second)
		{
			(*it).second((*it).first, data);
		}
	}
#endif
	std::string strId;
	std::string strCam;
	bool bRet1 = data.cId.SerializeToString(&strId);
	bool bRet2 = data.cCam.SerializeToString(&strCam);
	if (bRet1 == true && bRet2 == true)
	{
		emit(SignalCallChange(data.type, strId, strCam));
	}
	return true;
}

inline bool StorFactory::OnEvent(VidEvent &pEvent, VidStor &pStor)
{	
	std::string strEvent;
	std::string strStor;

	pEvent.set_strstorname(pStor.strname());
	pEvent.set_strstorid(pStor.strid());
	bool bRet = pEvent.SerializeToString(&strEvent);
	if (bRet == true)
	{
		VDC_DEBUG( "%s  Get Event %d %s  %s bsearched %d\n",__FUNCTION__, 
								pEvent.strtime().c_str(), 
								pEvent.strdevicename().c_str(), pEvent.strtype().c_str(), 
								pEvent.bsearched());
		emit(SignalEvent(strEvent));
		if (pEvent.bsearched() == false)
		{
			emit(SignalEvent1());
		}
	}
	return true;
}

inline bool StorFactory::AddCam(astring strStorId, VidCamera &pParam)
{
	if (m_pConf.FindStor(strStorId) && m_StorClientMap[strStorId])
	{
		return m_StorClientMap[strStorId]->AddCam(pParam);
	}
	return false;
}

inline bool StorFactory::PtzCmd(astring strStorId, astring strId, u32 action, double param)
{
	if (m_pConf.FindStor(strStorId) && m_StorClientMap[strStorId])
	{
		return m_StorClientMap[strStorId]->PtzCmd(strId, action, param);
	}
	return false;
}

inline bool StorFactory::DeleteCam(astring strStorId, astring strId)
{
	if (m_pConf.FindStor(strStorId) && m_StorClientMap[strStorId])
	{
		return m_StorClientMap[strStorId]->DeleteCam(strId);
	}
	return false;
}


inline bool StorFactory::SearchEvent(astring strStorId, astring strId, s64 nStart, s64 nEnd)
{
	if (m_pConf.FindStor(strStorId) && m_StorClientMap[strStorId])
	{
		return m_StorClientMap[strStorId]->SearchEvent(strId, nStart, nEnd);
	}
	return false;
}

inline bool StorFactory::RegRealEvent(astring strStorId)
{
	if (m_pConf.FindStor(strStorId) && m_StorClientMap[strStorId])
	{
		return m_StorClientMap[strStorId]->RegRealEvent();
	}
	return false;
}

inline bool StorFactory::UnRegRealEvent(astring strStorId)
{
	if (m_pConf.FindStor(strStorId) && m_StorClientMap[strStorId])
	{
		return m_StorClientMap[strStorId]->UnRegRealEvent();
	}
	return false;
}

inline bool StorFactory::HandleEvent(astring strStorId, astring strId)
{
	if (m_pConf.FindStor(strStorId) && m_StorClientMap[strStorId])
	{
		return m_StorClientMap[strStorId]->HandleEvent(strId);
	}
	return false;
}

inline StorClientOnlineMap StorFactory::GetVidCameraOnlineList(astring strStor)
{
	StorClientOnlineMap empty;

	if (m_pConf.FindStor(strStor) && m_StorClientMap[strStor])
	{
		return m_StorClientMap[strStor]->GetVidCameraOnlineList();
	}

	return empty;
}

inline StorClientRecMap StorFactory::GetVidCameraRecList(astring strStor)
{
	StorClientOnlineMap empty;

	if (m_pConf.FindStor(strStor) && m_StorClientMap[strStor])
	{
		return m_StorClientMap[strStor]->GetVidCameraRecList();
	}

	return empty;
}

inline VidCameraList StorFactory::GetVidCameraList(astring strStor)
{	
	VidCameraList empty;
	
	if (m_pConf.FindStor(strStor) && m_StorClientMap[strStor])
	{
		return m_StorClientMap[strStor]->GetVidCameraList();
	}

	return empty;
}

inline astring StorFactory::GetVidCameraName(astring strStor, astring strCam)
{
	astring empty;
	if (m_pConf.FindStor(strStor) && m_StorClientMap[strStor])
	{
		return m_StorClientMap[strStor]->GetVidCameraName(strCam);
	}

	return empty;
}

inline bool StorFactory::GetOnline(astring strStor)
{
	if (m_pConf.FindStor(strStor) &&  m_StorClientMap[strStor])
	{
		return m_StorClientMap[strStor]->GetOnline();
	}
	return false;
}

inline void StorFactory::run()
{
	while(1)
	{
		ve_sleep(1000 * 90);
	}
#if 0

	CameraParamMap paramMap;
	/* Create the thread to update the Disk status */
	while (1)
	{
		paramMap.clear();
		{
			/* Got all the camera param */
			Lock();
			CameraMap::iterator it = m_CameraMap.begin(); 
			for(; it!=m_CameraMap.end(); ++it)
			{	
				s32 nIndex = (*it).first;
				CameraParam pParam;
				Camera *pCamera = m_CameraMap[nIndex];
				if (pCamera == NULL)
				{
					continue;//TODO
				}
				pCamera->GetCameraParam(pParam);
				paramMap[nIndex] = pParam;
			}
			UnLock();
		}
		{
			/* Loop all the cameraparam */
			CameraParamMap::iterator it = paramMap.begin(); 
			for(; it!=paramMap.end(); ++it)
			{	
				/* Loop to check the camera and update the url */
				s32 nIndex = (*it).first;
				(*it).second.m_wipOnline = (*it).second.CheckOnline();
				if ((*it).second.m_OnlineUrl == FALSE)
				{
					(*it).second.m_wipOnlineUrl = (*it).second.UpdateUrl();
			
					if ((*it).second.m_wipOnlineUrl == FALSE)
					{
						(*it).second.m_wipOnline = FALSE;
					}
					
				}
			}
		}
		{
			/* Loop all the cameraparam result and set to camera */
			CameraParamMap::iterator it = paramMap.begin(); 
			for(; it!=paramMap.end(); ++it)
			{	
				/* Loop to check the camera and update the url */
				s32 nIndex = (*it).first;
				Lock();
				CameraMap::iterator it1 = m_CameraMap.find(nIndex), 
							ite1 = m_CameraMap.end();

				if (it1 == ite1) 
				{
					/* the id may be delete */
					UnLock();
					continue;
				}

				CameraStatus bCheck = m_CameraMap[nIndex]->CheckCamera(
					(*it).second.m_strUrl, (*it).second.m_strUrlSubStream, 
					(*it).second.m_bHasSubStream, 
					(*it).second.m_wipOnline, (*it).second.m_wipOnlineUrl);
				
				StorFactoryCameraChangeData change;
				change.id = nIndex;
				switch (bCheck)
				{
					case DEV_OFF2ON:
					{
						change.type = STOR_FACTORY_CAMERA_ONLINE;
						m_CameraOnlineMap[nIndex] = 1;
						UnLock(); 
						CallCameraChange(change);
						Lock();
						break;
					}
					case DEV_ON2OFF:
					{
						change.type = STOR_FACTORY_CAMERA_OFFLINE;
						m_CameraOnlineMap[nIndex] = 0;
						UnLock(); 
						CallCameraChange(change);
						Lock();
						break;
					}
					default:
					{

						break;
					}
				}
				UnLock();
			}
		}
		ve_sleep(1000 * 90);
	}
#endif
	
}




#endif // __VSC_STOR_FACTORY_H_
