#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>


#include <k8055.h>


#define STAT_UNKNOWN	0
#define STAT_OPEN 		1
#define STAT_CLOSED 	2

#define OP_ROOF_OSC		2
#define OP_HATCH_UP		3
#define OP_HATCH_DOWN	4

void generate_web_page(const char *msg)
{
	FILE *fp = fopen("/var/www/index.html","w");
	if (!fp)
	{
		return;
	}

    time_t now;
    time(&now);

    struct tm* now_tm;
    now_tm = localtime(&now);

    char date[80];
    strftime (date, 80, "Now it's %Y-%m-%d %H:%M:%S.", now_tm);


    fprintf(fp,"<html><body>\n");
	fprintf(fp,"<h1 style=\"color: #5e9ca0;\">Burr Point Astro</h1>\n");
	fprintf(fp,"<h2 style=\"color: #2e6c80;\">Roof control</h2>\n");
	fprintf(fp,"<h3 style=\"color: #5e9ca0;\">%s</h3>\n",msg);
	fprintf(fp,"<h3 style=\"color: #5e9ca0;\">%s</h3>\n",date);
	fprintf(fp,"</html></body>\n");
	fclose(fp);
}
void nap(void)
{
	usleep(10000);
}

int cleanup(void)
{
	return CloseDevice();
}

void SetOutput(long op)
{
	while (SetDigitalChannel(op) <0)
	{
		nap();
	}
}

void ClearOutput(long op)
{
	while (ClearDigitalChannel(op) <0)
	{
		nap();
	}
}

int get_hatch_status()
{
	int up_status = ReadDigitalChannel(3);
	int down_status = ReadDigitalChannel(4);
		
	if ((up_status == 0) && (down_status == 1))
	{
		return STAT_CLOSED;
	}
	else if ((up_status == 1) && (down_status == 0))
	{
		return STAT_OPEN;
	}
	return STAT_UNKNOWN;
}

bool button_pressed()
{
	int but = ReadDigitalChannel(5);
	if (but == 1 )
	{
		return true;
	}
	return false;
}

int get_roof_status(void)
{
	int op_status = ReadDigitalChannel(1);
	int cl_status = ReadDigitalChannel(2);
		
	printf("roof status %d %d\n",op_status,cl_status);
	
	if ((op_status == 0) && (cl_status == 1))
	{
		return STAT_CLOSED;
	}
	else if ((op_status == 1) && (cl_status == 0))
	{
		return STAT_OPEN;
	}
	return STAT_UNKNOWN;
}

void WaitforHatch(int state)
{
	int state_count = 0;
	while(state_count < 3)
	{
		if (get_hatch_status() == state)
		{
			state_count ++;
		}
		else
		{
			state_count = 0;
		}
		sleep(1);
	}
}

void WaitForRoof(int state)
{
	int state_count = 0;
	while(state_count < 3)
	{
		if (get_roof_status() == state)
		{
			state_count ++;
		}
		else
		{
			state_count = 0;
		}
		sleep(1);
	}
}

void open_roof()
{
	generate_web_page("Opening hatch");
	SetOutput(OP_HATCH_UP);
	WaitforHatch(STAT_OPEN);
	ClearOutput(OP_HATCH_UP);
	generate_web_page("Hatch open");
	generate_web_page("Opening roof");
	SetOutput(OP_ROOF_OSC);
	sleep(1);
	ClearOutput(OP_ROOF_OSC);
	WaitForRoof(STAT_OPEN);
	generate_web_page("Roof Open");
}

void close_roof()
{
	generate_web_page("Closing roof");
	SetOutput(OP_ROOF_OSC);
	sleep(1);
	ClearOutput(OP_ROOF_OSC);
	WaitForRoof(STAT_CLOSED);
	generate_web_page("Roof Closed");
	generate_web_page("Closing hatch");
	SetOutput(OP_HATCH_DOWN);
	WaitforHatch(STAT_CLOSED);
	ClearOutput(OP_HATCH_DOWN);
	generate_web_page("Hatch closed");
}

int main(int argc, char **argv)
{
	int ret = OpenDevice(0);
	if (ret < 0)
	{
		generate_web_page("Failed to open k8055 device");
		return 	1;
	}
	
	generate_web_page("k8055 address 0 opened\n");
	ClearAllDigital();

	while(1==1)
	{
		while(button_pressed() == false)
		{
			nap();
		}
		printf("button pressed\n");
		if ((get_roof_status() == STAT_CLOSED) && (get_hatch_status() == STAT_CLOSED))
		{
			open_roof();
		}
		else
		{
			close_roof();
		}		
		
	}
	return 0;
}

