/******************************************************************************
 * Copyright 2013-2014 
 *
 * FileName:TcpServer.c
 *
 * Description: 
 *
 * Modification history:
 *     2014/12/1, v1.0 create this file.
*******************************************************************************/
#include "esp_common.h"

#include "espressif/esp8266/ets_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/timers.h"

#include "lwip/sys.h"
#include "lwip/sockets.h"

#include "Led.h"

xTaskHandle pvTcpServerThreadHandle;

int StartUp( uint16_t port )
{
	int sock_fd = 0;
	struct sockaddr_in name;

	sock_fd = socket( PF_INET, SOCK_STREAM, 0 );
	if (sock_fd == -1)
	{
		return -1;
	}
	
	memset( &name, 0, sizeof( name ) );
	
	name.sin_family = AF_INET;
	name.sin_port = htons(port);
	name.sin_addr.s_addr = htonl(INADDR_ANY);
	if ( bind( sock_fd, (struct sockaddr *)&name, sizeof(name)) < 0 )
	{
		return -1;
	}

	if ( port == 0)  /* if dynamically allocating a port */
	{
		return -1;
	}
	if ( listen(sock_fd, 5) < 0 )
	{
		return -1;
	}
	
	return(sock_fd);	
}

/**
	* @brief  no .	  
	* @note   no.
	* @param  no.
	* @retval no.
	*/

void tcp_server_thread( void *pvParameters )
{
	char *ReadBuf = NULL;
	int ReadLen;
	int server_sock = -1, client_sock = -1 ;
	
	struct sockaddr_in client_name;
	int client_name_len = sizeof( client_name );

	STATION_STATUS StaStatus;
	do
	{
		StaStatus = wifi_station_get_connect_status( );

		vTaskDelay( 1000 / portTICK_RATE_MS );
	}
	while( StaStatus != STATION_GOT_IP );

	ReadBuf = malloc( 1460 );
		
	if( ReadBuf == NULL )
	{
		vTaskDelete(NULL);
		return;
	}
	server_sock = StartUp( 88 );

	if( server_sock == -1 )
	{
		printf("Tcp server startup fail!\r\n");

		vTaskDelete(NULL);
		return;
	}

	for( ;; )
	{
		client_sock = accept( server_sock , (struct sockaddr *)&client_name , &client_name_len );

		if( client_sock != -1)
		{
			printf("accept socket!\r\n");
			for( ;; )
			{
				ReadLen = recv( client_sock , ReadBuf , 1077 , 0 );
				printf("tcp rcv len:%d!\r\n" , ReadLen );
				if( ReadLen > 0 )
				{
					ReadBuf[ReadLen] = '\0';
					send( client_sock , ReadBuf , ReadLen , 0 );
				}
				else
				{
					if( ( ReadLen < 0 )&& ( errno != EAGAIN ) )
					{
						break;
					}
					else if( ReadLen == 0 )
					{
						break;
					}
				}
			}
		}

		close( client_sock );
		printf("close socket!\r\n");
	}
}

/**
	* @brief  no .	  
	* @note   no.
	* @param  no.
	* @retval no.
	*/
void tcp_server_thread_init( void )
{
	pvTcpServerThreadHandle = sys_thread_new("tcp_server" ,  tcp_server_thread , NULL, 256 , 5 );
	if( pvTcpServerThreadHandle != NULL )
	{
		printf("tcp_server Created!\r\n"  );
	}
}

