#ifndef TT_CGI_H
#define TT_CGI_H 1

/* show errors if dependencies are not included */

#if !LWIP_HTTPD_CUSTOM_FILES
#error This needs LWIP_HTTPD_CUSTOM_FILES
#endif

#if !LWIP_HTTPD_DYNAMIC_HEADERS
#error This needs LWIP_HTTPD_DYNAMIC_HEADERS
#endif

#if !LWIP_HTTPD_CGI
#error This needs LWIP_HTTPD_CGI
#endif

/* endpoints */
#define PROJECT_HOME_ENDPOINT "/home.html"
#define PROJECT_MONITORING_ENDPOINT "/monitoring.html"
#define PROJECT_LIMITS_ENDPOINT "/limits.html"
#define PROJECT_FANS_CONTROL_ENDPOINT "/fans_control.html"
#define API_MONITORING_DATA "/api/monitoring-data"
#define API_LIMITS_DATA "/api/limits-data"
#define API_FANS_STATUS_DATA "/api/fans-status-data"
#define ERROR_404_ENDPOINT "/404.html"

/* setters */
#define SET_LIMIT_ENDPOINT "/set-limit"
#define SET_FANS_ENDPOINT "/set-fans"
#define SET_MANUAL_CONTROL_ENDPOINT "/set-manual-control"

/* getters */
#define GET_DHT22_DATA_ENDPOINT "/dht22.json"
#define GET_LIMITS_ENDPOINT "/limits.json"


/* initialization functions */
void custom_files_init(void);
void cgi_init(void);

#endif
