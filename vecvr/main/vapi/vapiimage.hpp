#ifndef __VE_IMAGE_API_H__
#define __VE_IMAGE_API_H__

#include "CivetServer.h"
#include "server/factory.hpp"
#include "oapi/oapis.hpp"
#include "cppkit/ck_string.h"
#include "cppkit/os/ck_sprintf.h"
#include "rapidmedia/rapidmedia.hpp"
#include "vplay.hpp"


#ifdef WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif
using namespace cppkit;

#define VAPI_JPEG_SIZE 1024*1024 * 1

/* /vapi/GetImage?Camera=xxxxxxxx(guid)&Width=xx&Heigh=xx */
class WebAPIGetImageHandler : public CivetHandler
{
public:
	WebAPIGetImageHandler(Factory &pFactory)
		:m_pFactory(pFactory)
	{

	}
private:
	bool
	handleAll(const char *method,
	          CivetServer *server,
	          struct mg_connection *conn)
	{
		u8 * pBuf=NULL;
		int nLen= 0;
		std::string strCamera = "";
		int w = 0;
		int h = 0;
		if (CivetServer::getParam(conn, "Camera", strCamera)  == true)
		{
			/* Get Current I frame */
			astring strWidth, strHeigh;
			if (CivetServer::getParam(conn, "Width", strWidth))
			{
				ck_string cktmp = strWidth;
				w = cktmp.to_uint16(10);
			}

			if (CivetServer::getParam(conn, "Heigh", strHeigh))
			{
				ck_string cktmp = strHeigh;
				h = cktmp.to_uint16(10);
			}

			VideoFrame frame;
			RawFrame rawFrame;
			memset(&frame, 0, sizeof(frame));
			memset(&rawFrame, 0, sizeof(rawFrame));
			if (m_pFactory.GetiFrame(strCamera, frame) == TRUE)
			{
				/* Get the iFrame */
				void * pDecoder;
				if (RapidMedia::DecodeiFrame(frame, rawFrame, pDecoder) == TRUE)
				{
					/* Encode to JPEG */
					pBuf = (uint8_t*)malloc(VAPI_JPEG_SIZE);

					nLen = VPlay::EncodeJPEG(rawFrame, w, h, pBuf, VAPI_JPEG_SIZE);
					printf("get len %d\n", nLen);

					RapidMedia::DecodeiFrameFreeDec(pDecoder);
				}
				free(frame.dataBuf);
			}
		}

		if (pBuf != NULL && nLen > 0)
		{
			std::string s = "";
			mg_printf(conn,
			          "HTTP/1.1 200 OK\r\nContent-Type: "
			          "image/jpeg\r\n"
					  "Content-Length: %d\r\n\r\n", nLen);
			image_write_raw(pBuf, nLen, "test.jpg");
		
			mg_write(conn, (const char *)pBuf, nLen);
		}else
		{
			mg_printf(conn,
			          "HTTP/1.1 200 OK\r\nContent-Type: "
			          "text/plain\r\nConnection: close\r\n\r\n");
			mg_printf(conn, "Can't Get the Image!\n");
		}

		if (pBuf)
		{
			free(pBuf);
			pBuf = NULL;
		}
		return true;
	}

  public:
	bool
	handleGet(CivetServer *server, struct mg_connection *conn)
	{
		return handleAll("GET", server, conn);
	}
	bool
	handlePost(CivetServer *server, struct mg_connection *conn)
	{
		return handleAll("POST", server, conn);
	}
private:
	Factory &m_pFactory;
};


#endif /* __VE_WEB_SERVER_H__ */
