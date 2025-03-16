#include "mpshell/inc/mpshell.h"

bool RUN                  = true;
char *host                = NULL;
char *port_string         = NULL;
int port                  = -1;
int protocol              = PROTOCOL_TCP;
extern uint16_t max_payload_size;
extern DWORD sleep_size;
extern time_t timeout;

int main(int argc, const char *argv[])
{
    char delimiter[] = "=";
    for (int i = 1; i < argc; i++)
    {
        char *param_name  = NULL;
        char *param_value = NULL;

        char *current_argument = (char *) argv[i];
        param_name = strtok(current_argument, delimiter);

        if (param_name != NULL)
            param_value = strtok(NULL, delimiter);

        if (param_name != NULL && param_value != NULL)
        {
            if (strcmp(param_name, "--host") == 0)               
            { 
                host = param_value; 
            }
            else if (strcmp(param_name, "--port") == 0)         
            { 
                port = atoi(param_value); 
                port_string = param_value; 
            }
            else if (strcmp(param_name, "--protocol") == 0)
            {
                if (strcmp(param_value, "tcp") == 0)             
                { 
                    protocol = PROTOCOL_TCP; 
                }
                else if (strcmp(param_value, "udp") == 0)        
                { 
                    protocol = PROTOCOL_UDP; 
                }
                else if (strcmp(param_value, "icmp") == 0)       
                { 
                    protocol = PROTOCOL_ICMP; 
                }
                else 
                { 
                    show_usage(true); 
                }
            }
            else if (strcmp(param_name, "--payload-size") == 0)  
            { 
                max_payload_size = (size_t) atoi(param_value); 
            }
            else if (strcmp(param_name, "--sleep") == 0)        
            { 
                sleep_size       = (DWORD) (atoi(param_value)); 
            }
            else if (strcmp(param_name, "--timeout") == 0)       
            { 
                timeout          = (time_t) (atoi(param_value)); 
            }
            else 
            { 
                show_usage(true); 
            }
        }
        else 
        { 
            show_usage(true); 
        }
    }

    if (host == NULL || strlen(host) == 0)           
    { 
        show_usage(true); 
    }
    else if ((protocol != PROTOCOL_ICMP) && (port <= 0 || port > 65535))  
    { 
        show_usage(true); 
    }
    else if ((protocol != PROTOCOL_TCP) && (sleep_size <= 0))     
    { 
        show_usage(true); 
    }
    else if ((protocol != PROTOCOL_TCP) && (max_payload_size <=0 || max_payload_size > DEFAULT_MAX_PAYLOAD_SIZE)) 
    { 
        show_usage(true); 
    }
    else
    {
        init_buffers();
        int status = EXIT_SUCCESS;
        switch (protocol)
        {
            case PROTOCOL_TCP:  
                status = open_tcp_channel(host, port_string); 
                break;
            case PROTOCOL_UDP:  
                status = open_udp_channel(host, port); 
                break;
            case PROTOCOL_ICMP: 
                status = open_icmp_channel(host);
        }

        cleanup();
        return status;
    }
}
