/**************************************************************************
 * ������ ���������� ������������� ��� ������ � RTC, � ������ � Unix Time *
 * ��������� ���������� �� �������� � ���������(����, �����, ���) 		  *
 * � ����� (����, ������, �������), � �������							  *
 **************************************************************************/
#define SEC_A_DAY 86400
typedef struct
	{
	int year;
	char mon;
	char mday;
	char hour;
	char min;
	char sec;
	char wday;
	} unix_cal;

void timer_to_cal (unsigned long timer, unix_cal * unix_time);
unsigned long cal_to_timer (unix_cal * unix_time);
