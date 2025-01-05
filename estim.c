/*
E-Stim Command Line Tool for 2B
Author: AlexRope <https://fetlife.com/users/4887759>
*/

#include "estim.h"

const char* STR_MODES[] = { "PULSE", "BOUNCE", "CONTINUOUS", "A_SPLIT", "B_SPLIT", "WAVE", "WATERFALL", "SQUEEZE", "MILK", "THROB", "THRUST", "RANDOM", "STEP", "TRAINING", "MICROPHONE", "STEREO", "TICKLE" };
const char* STR_JOINED[] = { "CHANNELS_UNLINKED", "CHANNELS_LINKED" };

int NUM_MODES=sizeof(STR_MODES)/sizeof(STR_MODES[0]);
int DEBUG=0;
int fd=0;

Status status;

int printDebugString(unsigned char* buf, int len) {
	int i;
	for(i=0;i<len;i++) {
		if(buf[i] == 0xd) printf("\\r");
		else if(buf[i] == 0xa) printf("\\n");
		else printf("%c",buf[i]);
	}
	printf(" (");
	for(i=0;i<len;i++) {
		printf("0x%x",buf[i]);
		if(i != len-1) printf(" ");
	}
	printf(")\n");
}

int set_interface_attribs(int fd, int speed) {
	struct termios tty;

	if (tcgetattr(fd, &tty) < 0) {
		printf("Error from tcgetattr: %s\n", strerror(errno));
		return -1;
	}

	cfsetospeed(&tty, (speed_t)speed);
	cfsetispeed(&tty, (speed_t)speed);

	tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
	tty.c_cflag &= ~CSIZE;
	tty.c_cflag |= CS8;         /* 8-bit characters */
	tty.c_cflag &= ~PARENB;     /* no parity bit */
	tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
	tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

	/* setup for non-canonical mode */
	//tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON); /* turn off s/w flow control */
	//tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN); /* make raw */
	//tty.c_oflag &= ~OPOST; /* make raw */

	tty.c_cc[VMIN] = 1; // Non-blocking read
	tty.c_cc[VTIME] = 1; // 0.5 second read timeout.

	// Make Raw
	cfmakeraw(&tty);
	
	// Flush then apply settings.
	tcflush(fd, TCIFLUSH);
	if (tcsetattr(fd, TCSANOW, &tty) != 0) {
		printf("Error from tcsetattr: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

Status responseListen(const int fd) {
	unsigned char attrBuf[10] = {0};
	unsigned char buf[1];
	int fin=0;
	int attrIndex=0;
	int attrCount=0;
	Status status;
	
	if(DEBUG) printf("Received:\n");

	while (!fin) {	
		int rdlen = read(fd, buf, 1);
		if (rdlen > 0) {
			if (buf[0] == 0x3a || buf[0] == 0x0a) {
				//This is the end of the attribute, append a null terminator.
				attrBuf[attrCount] = '\0';
				
				if (buf[0] == 0x0a) {
					fin = 1;
				}
				
				if (DEBUG) {
					printf("attrBuf = ");
					printDebugString(attrBuf, 10);
				}

				int value;
				unsigned char tmpBuf[10];
				strncpy(tmpBuf, attrBuf, 10);
				
				if (attrIndex < 6 || attrIndex == 7) {
					value = strtol(tmpBuf,NULL,10) & 0xFF;
				}

				switch(attrIndex) {
				case 0: status.battery = value; break;
				case 1: status.channel_a = value/2; break;
				case 2: status.channel_b = value/2; break;
				case 3: status.duty_cycle = value/2; break;
				case 4: status.feeling = value/2; break;
				case 5: status.mode = value; break;
				case 6: status.power = attrBuf[0]; break;
				case 7: status.joined = value; break;
				case 8: strcpy(status.version, attrBuf); break;
				}

				attrCount=0;
				attrIndex++;
				memset(attrBuf, 0, sizeof(attrBuf));
			}
			else {
				attrBuf[attrCount++] = buf[0];
			}
		} else if (rdlen < 0) {
			printf("Error from read: %d: %s\n", rdlen, strerror(errno));
		}
	}
	
	return status;
}

int writeOut(unsigned char *buf, int len) {
	if (DEBUG) {
		printf("Writing (%d): ", len);
		printDebugString(buf,len);	
	}

	int wlen = write(fd, buf, len);
	if (wlen != len) {
		printf("Write error: %d, %d\n", wlen, errno);
	}
	tcdrain(fd);

	if (DEBUG) {
		printf("Sent (%d): ", wlen);
		printDebugString(buf, wlen);
	}

	status = responseListen(fd);
}

void print_color_percentage(int percentage) {
	if(percentage > 66) printf("\e[31m%d\%\e[0m\n",percentage);
	else if(percentage > 33) printf("\e[33m%d\%\e[0m\n",percentage);
	else if(percentage > 0) printf("\e[32m%d\%\e[0m\n",percentage);
	else printf("%d\%\n",percentage);
}

void print_color_power(char power) {
	if(power == 'L') printf("\e[32mLOW\e[0m\n");
	else if(power == 'H') printf("\e[31mHIGH\e[0m\n");
	else printf("%c\n",power);
}

void print_status(Status* status) {	
	printf("\e[1;4m- STATUS -\n\e[0m");
	printf("\e[1mBattery:\e[0m %d\n\e[0m", status->battery);
	printf("\e[1mChannel A Intensity:\e[0m ");
	print_color_percentage(status->channel_a);
	printf("\e[1mChannel B Intensity:\e[0m ");
	print_color_percentage(status->channel_b);
	printf("\e[1mDuty Cycle:\e[0m %d\%\n\e[0m", status->duty_cycle);
	printf("\e[1mFeeling:\e[0m %d\%\n\e[0m", status->feeling);
	printf("\e[1mMode:\e[0m %s\n\e[0m", STR_MODES[status->mode]);
	printf("\e[1mPower:\e[0m ");
	print_color_power(status->power);
	printf("\e[1mJoined:\e[0m %s\n\e[0m", STR_JOINED[status->joined]);
	printf("\e[1mVersion:\e[0m %s\n\e[0m", status->version);
}

int setup_serial(char *portname) {
	int fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
	if (fd < 0) {
		printf("Error opening %s: %s\n", portname, strerror(errno));
		return -1;
	}
	/*baudrate 9600, 8 bits, no parity, 1 stop bit */
	set_interface_attribs(fd, B9600);
	//set_mincount(fd, 0);                /* set to pure timed read */
	return fd;
}

int set_channel_level(channel_t channel, int out) {
	char newOut[5];
	int n;

	if (out < 0 || out > 100) {
		printf("\e[1mError: Channel output must be between 0 and 100.\e[0m\n");
		return 0;
	}

	switch(channel) {
	case CHANNEL_A: n=sprintf(newOut, "A%d\r", out); break;
	case CHANNEL_B: n=sprintf(newOut, "B%d\r", out); break;
	}

	writeOut(newOut, n);
}

int set_feeling(int feeling) {
	char newOut[5];
	int n;

	if (feeling < 1 || feeling > 100) {
		printf("\e[1mError: Feeling must be between 1 and 100.\e[0m\n");
		return 0;
	}

	n=sprintf(newOut, "D%d\r", feeling);

	writeOut(newOut, n);
}

int set_duty_cycle(int duty_cycle) {
	char newOut[5];
	int n;

	if (duty_cycle < 2 || duty_cycle > 100) {
		printf("\e[1mError: Rate must be between 2 and 100.\e[0m\n");
		return 0;
	}

	n=sprintf(newOut, "C%d\r", duty_cycle);
	writeOut(newOut, n);
}

void set_power(power_t power) {
	switch(power) {
	case POWER_LOW: writeOut("L\r", 2); break;
	case POWER_HIGH: writeOut("H\r", 2); break;
	}
}

void set_joined(joined_t joined) {
	switch(joined) {
	case CHANNELS_UNLINKED: writeOut("J0\r", 3); break;
	case CHANNELS_LINKED: writeOut("J1\r", 3); break;
	}
}

void send_kill() {
	writeOut("K\r", 2);
}

void send_reset() {
	writeOut("E\r", 2);
}

int set_mode(emode_t mode) {
	if (mode < 0 || mode > NUM_MODES-1) {
		printf("\e[1mError: Mode must be between 0 and %d. (%d)\e[0m\n", NUM_MODES-1, mode);
		return 0;
	}
	
	char newMode[5];
	int n=sprintf(newMode, "M%d\r", mode);
	writeOut(newMode, n);
}

void get_status(int echo) {
	writeOut("\r", 1);
	if (echo) print_status(&status);
}

void print_help(char filename[]){
	printf("usage: %s {-h} {-v} {-k} {-r} -i DEVICE -a CHANNEL_A_INTENSITY -b CHANNEL_B_INTENSITY -f FEELING_LEVEL -d DUTY_CYCLE -m MODE -p POWER -j JOINED\n", filename);
	printf(" -i DEVICE      Serial tty port that your E-STIM 2B is connected. (e.g. /dev/ttyUSB0)");
	printf(" -h             Print this help message\n");
	printf(" -v             Debug logging\n");
	printf(" -k             Kill Channel A and B output.\n");
	printf(" -r             Reset E-Stim to default startup settings.\n");
	printf(" -a CHANNEL_A_INTENSITY Set channel A intensity (values: 0 - 100)\n");
	printf(" -b CHANNEL_B_INTENSITY Set channel B intensity (values: 0 - 100)\n");
	printf(" -f FEELING_LEVEL	Set feeling level (values: 2 - 100)\n");
	printf(" -d DUTY_CYCLE		Set duty cycle (values: 2 - 100)\n");
	printf(" -m MODE		Set Mode\n");
	printf(" -p POWER		Set Power (values: 0 or 1)\n");
	printf(" -j JOINED		Set Joined (values: 0 or 1)\n");
}

int main(int argc, char * argv[]) {
	int channelA=-1;
	int channelB=-1;
	int duty_cycle=-1;
	int feeling=-1;
	int kill=-1;
	int reset=-1;
	emode_t mode=-1;
	power_t power=-1;
	joined_t joined=-1;
	char* device;
	
	int opt;
	while ((opt = getopt (argc, argv, "hvkra:b:d:f:m:p:j:i:")) != -1) {
		switch(opt) {
		case 'v': DEBUG=1; break;
		case 'k': kill=1; break;
		case 'r': reset=1; break;
		case 'h': print_help(argv[0]); return 0; break;
		case 'a': channelA=atoi(optarg); break;
		case 'b': channelB=atoi(optarg); break;
		case 'd': duty_cycle=atoi(optarg); break;
		case 'f': feeling=atoi(optarg); break;
		case 'm': mode=atoi(optarg); break;
		case 'p': power=atoi(optarg); break;
		case 'j': joined=atoi(optarg); break;
		case 'i': device=optarg; break;
		}
	}

	//Needs to be refactored to take device as user input or automatically detect.
	if ((fd = setup_serial(device)) < 0) {
		return 1;
	}
	
	//Get initial status.
	get_status(0);

	//Send kill if requested.
	if(kill > 0 || reset > 0) {
		if(kill > 0) send_kill();
		if(reset > 0) send_reset();
	}
	else {
		//Update settings	
		if(power != -1 && power != status.power) set_power(power);
		if(mode != -1 && mode != status.mode) set_mode(mode);
		if(duty_cycle != -1 && duty_cycle != status.duty_cycle) set_duty_cycle(duty_cycle);
		if(feeling != -1 && feeling != status.feeling) set_feeling(feeling);	
		if(joined != -1 && joined != status.joined) set_joined(joined);
		if(channelA != -1 && channelA != status.channel_a) set_channel_level(CHANNEL_A, channelA);
		if(channelB != -1 && channelB != status.channel_b) set_channel_level(CHANNEL_B, channelB);
	}

	//Print status
	get_status(1);
}
