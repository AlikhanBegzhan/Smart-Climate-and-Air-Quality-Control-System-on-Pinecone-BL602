/*
 * Implementation of CGI handlers and adoption of generated JSON to sensors use case.
 */

#include "lwip/apps/httpd.h"
#include "lwip/opt.h"

#include "lwip/apps/fs.h"
#include "lwip/def.h"
#include "lwip/mem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <bl_gpio.h>

#include "cJSON.h"

#include "cgi.h"

#include "dht22.h"
#include "mq2.h"
#include "relay.h"

// HTML files
#include "home_html.h"
#include "monitoring_html.h"
#include "limits_html.h"
#include "fans_control_html.h"

static const char *cgi_handler_set_limit(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
  printf("iIndex: %d, iNumParams: %d, pcParam: %s, pcValue: %s\r\n", iIndex, iNumParams, pcParam[0], pcValue[0]);
  if (iNumParams == 1)
  {
    // Data should be provided in tenths e.g. 20.5°C = 205, 60.5% = 605
    if (!strcmp(pcParam[0], "temp_limit_low"))
    {
      temperature_limit_low = strtol(pcValue[0], NULL, 10);
      return SET_LIMIT_ENDPOINT; // Return success URL
    }
    else if (!strcmp(pcParam[0], "temp_limit_high"))
    {
      temperature_limit_high = strtol(pcValue[0], NULL, 10);
      return SET_LIMIT_ENDPOINT; // Return success URL
    }
    else if (!strcmp(pcParam[0], "hum_limit_high"))
    {
      humidity_limit_high = strtol(pcValue[0], NULL, 10);
      return SET_LIMIT_ENDPOINT; // Return success URL
    }
  }

  // Return 404 for any errors
  return ERROR_404_ENDPOINT;
}

static const char *cgi_handler_set_fans(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    // Only allow fan control if manual control is enabled
    if (!manual_control) {
        return ERROR_404_ENDPOINT;
    }

    if (iNumParams == 2 && !strcmp(pcParam[0], "type") && !strcmp(pcParam[1], "state"))
    {
        if (!strcmp(pcValue[0], "cooling"))
        {
            cooling_fans_status = (pcValue[1][0] == '1');
            return SET_FANS_ENDPOINT;
        }
        else if (!strcmp(pcValue[0], "heater"))
        {
            heater_fan_status = (pcValue[1][0] == '1');
            return SET_FANS_ENDPOINT;
        }
    }
    return ERROR_404_ENDPOINT;
}

static const char *cgi_handler_set_manual_control(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    if (iNumParams == 1 && !strcmp(pcParam[0], "state"))
    {
        manual_control = (pcValue[0][0] == '1');
        if (!manual_control) {
            // Reset fan states when disabling manual control
            cooling_fans_status = false;
            heater_fan_status = false;
        }
        return SET_MANUAL_CONTROL_ENDPOINT;
    }
    return ERROR_404_ENDPOINT;
}

static const tCGI cgi_handlers[] = {
    {SET_LIMIT_ENDPOINT,
     cgi_handler_set_limit}, 
     {SET_FANS_ENDPOINT,
     cgi_handler_set_fans}, 
     {SET_MANUAL_CONTROL_ENDPOINT,
     cgi_handler_set_manual_control}};

int fs_open_custom(struct fs_file *file, const char *name)
{
    // Remove redundant allocations
    if (!strcmp(name, PROJECT_HOME_ENDPOINT) ||
        !strcmp(name, PROJECT_MONITORING_ENDPOINT) ||
        !strcmp(name, PROJECT_LIMITS_ENDPOINT) ||
        !strcmp(name, PROJECT_FANS_CONTROL_ENDPOINT)) {
        
        // Directly assign static HTML without allocation
        if (!strcmp(name, PROJECT_HOME_ENDPOINT)) {
            file->data = (const char *)home_html;
            file->len = home_html_len;
        } else if (!strcmp(name, PROJECT_MONITORING_ENDPOINT)) {
            file->data = (const char *)monitoring_html;
            file->len = monitoring_html_len;
        } else if (!strcmp(name, PROJECT_LIMITS_ENDPOINT)) {
            file->data = (const char *)limits_html;
            file->len = limits_html_len;
        } else {
            file->data = (const char *)fans_control_html;
            file->len = fans_control_html_len;
        }
        
        file->index = file->len;
        file->flags = FS_FILE_FLAGS_HEADER_PERSISTENT;
        return 1;
    }

    // For API endpoints, allocate only what's needed
    char *response = NULL;
    if (!strcmp(name, API_MONITORING_DATA) ||
        !strcmp(name, API_LIMITS_DATA) ||
        !strcmp(name, API_FANS_STATUS_DATA)) {
        
        cJSON *json = cJSON_CreateObject();
        if (!json) return 0;
        
        // Add JSON data based on endpoint
        if (!strcmp(name, API_MONITORING_DATA)) {
          cJSON_AddNumberToObject(json, "temperature", temperature / 10.0f);
          cJSON_AddNumberToObject(json, "humidity", humidity / 10.0f);
          cJSON_AddBoolToObject(json, "gas_detected", gas_detected);
          cJSON_AddNumberToObject(json, "cooling_fans_status", cooling_fans_status);
          cJSON_AddNumberToObject(json, "heater_fan_status", heater_fan_status);
          cJSON_AddNumberToObject(json, "temperature_limit_low", temperature_limit_low);
          cJSON_AddNumberToObject(json, "temperature_limit_high", temperature_limit_high);
          cJSON_AddNumberToObject(json, "humidity_limit_high", humidity_limit_high);
        } else if (!strcmp(name, API_LIMITS_DATA)) {
          cJSON_AddNumberToObject(json, "temperature_limit_low", temperature_limit_low);
          cJSON_AddNumberToObject(json, "temperature_limit_high", temperature_limit_high);
          cJSON_AddNumberToObject(json, "humidity_limit_high", humidity_limit_high);
        } else if (!strcmp(name, API_FANS_STATUS_DATA)) {
          cJSON_AddNumberToObject(json, "cooling_fans_status", cooling_fans_status);
          cJSON_AddNumberToObject(json, "heater_fan_status", heater_fan_status);
        }
        
        response = cJSON_PrintUnformatted(json);
        cJSON_Delete(json);
        
        if (!response) return 0;
        
        int response_size = strlen(response);
        file->pextension = mem_malloc(response_size + 1);
        
        if (!file->pextension) {
            free(response);
            return 0;
        }
        
        memcpy(file->pextension, response, response_size + 1);
        file->data = (const char *)file->pextension;
        file->len = response_size;
        file->index = response_size;
        file->flags = FS_FILE_FLAGS_HEADER_PERSISTENT;
        
        free(response);
        return 1;
    }
    
    return 0;
}

/* closing the custom file (free the memory) */
void fs_close_custom(struct fs_file *file)
{
  if (file && file->pextension)
  {
    mem_free(file->pextension);
    file->pextension = NULL;
  }
}

/* reading the custom file (nothing has to be done here, but function must be defined */
int fs_read_custom(struct fs_file *file, char *buffer, int count)
{
  LWIP_UNUSED_ARG(file);
  LWIP_UNUSED_ARG(buffer);
  LWIP_UNUSED_ARG(count);
  return FS_READ_EOF;
}

/* initialization functions */
void custom_files_init(void)
{
  printf("Initializing module for generating JSON output\r\n");
  /* Nothing to do as of now, should be initialized automatically */
}

void cgi_init(void)
{
  printf("Initializing module for CGI\r\n");
  http_set_cgi_handlers(cgi_handlers, LWIP_ARRAYSIZE(cgi_handlers));
}
