/******************************************************************************
 * Copyright 2013-2014 
 *
 * FileName: UdpServer.c
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

#include "lwip/sockets.h"


#define UDP_DATA_LEN          100

/**
	* @brief  no .	  
	* @note   no.
	* @param  no.
	* @retval no.
	*/
void UdpServerTask( void *pvParameters )
{
	int32 sock_fd , ret;
	struct sockaddr_in ServerAddr;
	struct sockaddr from;
	uint8_t fromlen;
	uint8 Mac[6];
	
	int32 nNetTimeout = 60000; 
	char udp_msg[ UDP_DATA_LEN ];

	uint8_t i = 0;
	
	do
	{
		if( wifi_get_macaddr( 0x00 , Mac ) == true )
		{
			break;
		}

		i++;
	}while( i < 3 );
		
	if( i == 3 )
	{
		system_restart();
	}

	STATION_STATUS StaStatus;

	do
	{
		StaStatus = wifi_station_get_connect_status( );

		vTaskDelay( 1000 / portTICK_RATE_MS );
	}
	while( StaStatus != STATION_GOT_IP );

	printf("Get IP Ok!\r\n");
	
	sock_fd = socket( AF_INET , SOCK_DGRAM , 0 );

	if( sock_fd == -1 )
	{
		printf("Udp server get socket fail!\r\n");

		vTaskDelete(NULL);
		return;
	}

	memset( &ServerAddr , 0 , sizeof( ServerAddr ) );
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_addr.s_addr = INADDR_ANY;
	ServerAddr.sin_port = htons( 6666 );           										// UDP���ط������˿�Ϊ6666.
	ServerAddr.sin_len = sizeof( ServerAddr );

	fromlen = sizeof( struct sockaddr_in );

	if( bind( sock_fd , ( struct sockaddr * )&ServerAddr , sizeof( ServerAddr ) ) != 0 )
	{
		printf("Udp server bind port fail!\r\n");
		
		vTaskDelete(NULL);
		return;
	}

	setsockopt( sock_fd ,SOL_SOCKET , SO_RCVTIMEO ,(char*)&nNetTimeout, sizeof(int) );
	
	for( ;; )
	{
		ret = recvfrom( sock_fd , (uint8_t *)udp_msg , UDP_DATA_LEN , 0 , (struct sockaddr *)&from , (socklen_t *)&fromlen );

		if( ret > 0 )
		{
			udp_msg[ret] = '\0';

			printf("Udp server recive msg:%s\r\n" , udp_msg );

			if( strcmp( udp_msg , "find ESP8266") == 0)
			{
//				strcpy( udp_msg , "I'm a ESP8266!");
				sprintf( udp_msg , "I'm a ESP8266,MAC=%02X%02X%02X%02X%02X%02X" , Mac[0], Mac[1], Mac[2], Mac[3], Mac[4], Mac[5] );
			}
			sendto( sock_fd , (uint8_t *)udp_msg , strlen( udp_msg ) , 0 , (struct sockaddr *)&from , fromlen );
		}
	}

	vTaskDelete(NULL);
}

/**
	* @brief  no .	  
	* @note   no.
	* @param  no.
	* @retval no.
	*/
	
void UdpServerInit( void )
{
	xTaskCreate(UdpServerTask, "UdpServerTask", 256, NULL, 4, NULL);
}

