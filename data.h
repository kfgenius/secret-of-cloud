//���� ������ �ʿ��� �� ���� ����
#define ZERO	0
#define ONE		1
#define TWO		2
#define THREE	3
#define FOUR	4
#define FIVE	5

#define FILE_MAX 3

#define ORGINAL_SCREEN_X 320
#define ORGINAL_SCREEN_Y 240
#define SCREEN_X 640
#define SCREEN_Y 480
#define SCREEN_BUFFER	"CommonBlank"

char* bgm[]={"Sound\\Hondon.mid","Sound\\Juraona.mid","Sound\\Book.mid","Sound\\Sword.mid","Sound\\Horn.mid","Sound\\6th.mid","Sound\\Title.mid","Sound\\Gamble.mid","Sound\\Battle.mid","Sound\\Fly.mid","Sound\\End.mid"};
char answer[9];
int answer_max;

bool activate = true;

#define _GetKeyState( vkey )	HIBYTE(GetAsyncKeyState( vkey )) && activate

//�������� ��ȭ
#define TRIBE_SKY			m_party.IsIt(12) || m_party.IsIt(11)
#define TRIBE_LIGHTNING		m_party.IsIt(13) || m_party.IsIt(11)

FILE* fp;

bool enter=false, esc=false, key_space=false;
bool key_left=false, key_right=false, key_up=false, key_down=false;
char global_buffer[1024];

char* fighter[10]={"�Ƶ��","Ŭ���","����","����","�̾�","���","���ı�","����","����","����"};
char* item_name[]={"����","������ ��","�¾��� �ð�","ǳ�� 10%���α�","������ ����","��������","�ູ�� ��","������ ��","������ �Ȱ�"};
char* cloud_name[]={"ȥ��","������","å","Į","��","������"};

HWND hwnd;
//����
HSNDOBJ Sound[100];

LPDIRECTSOUND       SoundOBJ = NULL;
LPDIRECTSOUNDBUFFER SoundDSB = NULL;
DSBUFFERDESC        DSB_desc;

BOOL SoundCard;
BOOL ReplayFlag;

//�̵� ����
BOOL _MidiPlay(char* pszMidiFN, BOOL bReplayFlag = TRUE)
{
    char szMCISendString[256];

    wsprintf(szMCISendString,"open %s type sequencer alias MUSIC", pszMidiFN);

    if ( mciSendString ( "close all", NULL, 0, NULL ) != 0 ) return ( FALSE );
    if ( mciSendString ( szMCISendString, NULL, 0, NULL ) != 0 ) return ( FALSE );
    if ( mciSendString ( "play MUSIC from 0 notify", NULL, 0, hwnd ) != 0) return(FALSE);

    ReplayFlag = bReplayFlag; 
    return TRUE;
}

BOOL _MidiStop()
{
    if ( mciSendString ( "close all", NULL, 0, NULL) != 0 ) return ( FALSE );
    return TRUE;
}

BOOL _MidiReplay()
{
    if ( mciSendString ( "play MUSIC from 0 notify", NULL, 0, hwnd) != 0 ) return ( FALSE );
    return TRUE;
}

void _Play( int num )
{
    if ( SoundCard ) SndObjPlay( Sound[num], NULL );
}

//���ڿ� ó�� �Լ�
char* StrAdd(char* msg, ...)
{
	strcpy(global_buffer,"");
	va_list ap;
	va_start(ap,msg);
	int max=strlen(msg);
	for(int i=0; i<max; i++)
	{
		if(msg[i]=='c')
		{
			strcat(global_buffer,va_arg(ap,char*));
		}
		else if(msg[i]=='d')
		{
			char int_buffer[10];
			itoa(va_arg(ap,int),int_buffer,10);
			strcat(global_buffer,int_buffer);
		}
	}

	return global_buffer;
}

//��ȣȭ�� ���� ���ڿ�
char CodeTo[76]="dyKp:8jP;[R^FIqN0WsTX4U`a52Z]1HnbuVvlh76\\ti=SArJ@>Q_wgkoB?LCcYD3<M9EmGxeOfz";

int get_char_num(char chr)
{
	int i;
	for(i=0; i<76; i++)
		if(CodeTo[i]==chr)break;

	return i;
}

//�ִ� �ּҰ�
int Max(int x, int y)
{
	if(x>y)return x;
		else return y;
}

int Min(int x, int y)
{
	if(x<y)return x;
		else return y;
}

//��� ����
void Pause()
{
	while(1)
	{
		if(_GetKeyState(VK_RETURN))
		{
			if(!enter)
			{
				enter=true;
				break;
			}
		}
		else enter=false;
	}
}

//320x240ȭ���� 2��� ���
void Render()
{
	RECT dest_rect, src_rect;
	SetRect(&src_rect, 0, 0, ORGINAL_SCREEN_X, ORGINAL_SCREEN_Y);
	SetRect(&dest_rect, 0, 0, SCREEN_X, SCREEN_Y);
	jdd->DrawStretchedPicture(backbuffer, SCREEN_BUFFER, &dest_rect, &src_rect);
	jdd->Render();
}

//���̵� �ƿ�
void FadeOut()
{
	JPictureInfo pi;
	pi.SetOpacity(0.2f);
	jdd->SetPictureInfo("Black",&pi);
	for(int i=0; i<10; i++)
	{
		for(int j=0; j<12; j++)
			jdd->DrawPicture(SCREEN_BUFFER,"Black",0,j*20,NULL);
		Render();
		Sleep(30);
	}
	pi.SetOpacity(1.0);
	jdd->SetPictureInfo("Black",&pi);
}

//ȭ��Ʈ �ƿ�
void WhiteOut()
{
	JPictureInfo pi;
	pi.SetOpacity(0.2f);
	jdd->SetPictureInfo("White",&pi);
	for(int i=0; i<10; i++)
	{
		for(int j=0; j<12; j++)
			jdd->DrawPicture(SCREEN_BUFFER,"White",0,j*20,NULL);
		Render();
		Sleep(30);
	}
	pi.SetOpacity(1.0);
	jdd->SetPictureInfo("White",&pi);
}

//Ű ó��
bool GetKeyRight()
{
	if(_GetKeyState(VK_RIGHT))
	{
		if(!key_right)
		{
			key_right=true;
			return true;
		}
	}
	else key_right=false;
	
	return false;
}

bool GetKeyLeft()
{
	if(_GetKeyState(VK_LEFT))
	{
		if(!key_left)
		{
			key_left=true;
			return true;
		}
	}
	else key_left=false;
	
	return false;
}

bool GetKeyDown()
{
	if(_GetKeyState(VK_DOWN))
	{
		if(!key_down)
		{
			key_down=true;
			return true;
		}
	}
	else key_down=false;
	
	return false;
}

bool GetKeyUp()
{
	if(_GetKeyState(VK_UP))
	{
		if(!key_up)
		{
			key_up=true;
			return true;
		}
	}
	else key_up=false;
	
	return false;
}

/*==============================
			����ġ ����
================================
200 ~ 239		������
*/
#define SW_VISIT_MAX	0
#define SW_GET_BALLOON	1
#define SW_FIRST_DIREVE	2
#define SW_START		3
#define SW_GAMBLE		4
#define SW_QUIT			5
#define SW_BARAMGIL		6
#define SW_EQUIP_EXAM	7
#define SW_LENT_END		8
#define SW_BUY_BALLOON	9
#define SW_GET_ITEM		10//~15
#define SW_FIRST_FAINY	16	
#define SW_GANG			17
#define SW_GANG_PARK	18
#define SW_GANG_HELL	19
#define SW_RE_PARK	20
#define SW_RE_HELL	21
#define SW_SHIP_BATTLE	22
#define SW_SET_SEED		23
#define SW_SEED_MONEY	24
#define SW_CLOUD_JEWEL	25
#define SW_GET_WATER	26
#define SW_RAIN			27
#define SW_WATCH		28
#define SW_FIND_TRUE	29
#define SW_FIRST_DOOR	30
#define SW_MONEY_BOX	31
#define SW_GET_FIRE		32
#define SW_ATTACK		33
#define SW_END_OCT		34

#define SW_ITEM			200//~239

/*==============================
			���� ����
================================
61 ~ 88		����
89			���� ��
90 ~ 99		���� ��ŷ
100 ~ 199	����
200 ~ 239	��� ������
*/
#define VAR_SPOT			0
#define VAR_MONEY			1
#define VAR_TIME			2//~5
#define VAR_SOLUTION		6
#define VAR_ABIL_PILOT		7
#define VAR_MODEL			8
#define VAR_BANUL_SPEED		9
#define VAR_LENT_FEE		10
#define VAR_NEW_RECORD		11//~60
#define VAR_PARTY			61//~88
#define VAR_PARTY_NUM		89
#define VAR_RANK			90//~99

#define VAR_HP				100//~109
#define VAR_TARGET			110
#define VAR_AIR				111
#define VAR_PARTY_ADD		113
#define VAR_ENEMY_PARTY		114//~119
#define VAR_ENEMY_PARTY_NUM	112
#define VAR_POW				120//~129
#define VAR_SPD				130//~139
#define VAR_DEATH			140
#define VAR_PRIZE			141

#define VAR_EQUIP			200//~239
#define VAR_EQUIP_LIST		240//~243

/*==============================
			��� ��ȣ
================================
0	�豸��
1	�ձ���
2	3��ź
3	�Ͻ���
4	���๰
5	16��
6	����
20	����
21	����
*/

///////////////////////////////////////////////////////////////////////////////
//����ġ�� ����
#define SWITCHES 250
#define VALUES 250

class CSW_VAR
{
public:
	bool sw[SWITCHES];
	int var[VALUES];

	void SetSwitch(int from, int to, bool s);
	void SetVar(int from, int to, int v);
	void InitSwitch(bool s);
	void InitVar(int v);
	void SetVars(int from, int to, ...);
	bool CmpVar(int n, int max, ...);
};

void CSW_VAR::SetSwitch(int from, int to, bool s)
{
	for(int i=from; i<=to; i++)sw[i]=s;
}

void CSW_VAR::SetVar(int from, int to, int v)
{
	for(int i=from; i<=to; i++)var[i]=v;
}

void CSW_VAR::InitSwitch(bool s)
{
	SetSwitch(0,SWITCHES-1,false);
}

void CSW_VAR::InitVar(int v)
{
	SetVar(0,VALUES-1,0);
}

void CSW_VAR::SetVars(int from, int to, ...)
{
	va_list ap;
	va_start(ap,to);
	int max=to-from+1;
	for(int i=0; i<max; i++)
		var[from+i]=va_arg(ap,int);
	va_end(ap);
}

bool CSW_VAR::CmpVar(int n, int max, ...)
{
	bool equal=false;

	va_list ap;
	va_start(ap,max);
	for(int i=0; i<max; i++)
	{
		if(var[n]==va_arg(ap,int))
		{
			equal=true;
			break;
		}
	}
	va_end(ap);

	return equal;
}

CSW_VAR m_sv;
/////////////////////////////////////////////////////
//����Ʈ ó�� Ŭ����
class CList
{
	int start, end;	//����Ʈ ���۰� ��
	int sp;			//������
public:
	CList(int st, int ed, int s);
	bool Insert(int no);
	bool Delete(int no);
	bool IsIt(int no);
	void Chagne(int v1, int v2);
};

CList::CList(int st, int ed, int s)
{
	start=st;
	end=ed;
	sp=s;
}

bool CList::Insert(int no)
{
	if(start+m_sv.var[sp]>end)return false;	//�� ��

	m_sv.var[start+m_sv.var[sp]]=no;
	m_sv.var[sp]++;
	return true;
}

bool CList::Delete(int no)
{
	if(m_sv.var[sp]<=0)return false;	//����Ʈ�� ����� ��

	bool result=false;
	int max=start+m_sv.var[sp];
	for(int i=start; i<max; i++)
	{
		if(m_sv.var[i]==no)	//��ġ�ϴ� ��ȣ�� ã�� �� ���� ���� ���
		{
			for(int j=i; j<max-1; j++)m_sv.var[j]=m_sv.var[j+1];
			m_sv.var[sp]--;
			result=true;
			break;			
		}
	}
	return result;
}

bool CList::IsIt(int no)
{
	int max=start+m_sv.var[sp];
	for(int i=start; i<max; i++)
	{
		if(m_sv.var[i]==no)return true;
	}

	return false;
}

void CList::Chagne(int v1, int v2)
{
	if(v1 < start || v1 > end || v2 < start || v2 > end)
	{
		puts("������ ��� �ٲٱ� ����� �־����ϴ�.");
		return;
	}

	int tmp;
	tmp=m_sv.var[v1];
	m_sv.var[v1]=m_sv.var[v2];
	m_sv.var[v2]=tmp;
}

CList m_party(VAR_PARTY,VAR_PARTY+27,VAR_PARTY_NUM);
CList m_party2(VAR_ENEMY_PARTY,VAR_ENEMY_PARTY+5,VAR_ENEMY_PARTY_NUM);

/////////////////////////////////////////////////////
//��ȭ ó�� Ŭ����
class CDlg
{
	int snrs;
	char** snr;
	int mark_no;
	int bookmark[1000];

	int TextPrint(char* content, int y, int back);
	int TextSelect(int y, int back, int start);
	void ShowBack(int back, int select=-1);

public:
	int TextSel(int n_dlg, int back=-1, int start=0);
	int TextSel(char* input_dlg, int back=-1, int start=0);
	void TextSnr(int from, int back=-1, bool erase=true);
	void TextPut(char* content, int back=-1);

	CDlg();
	~CDlg();
};

#define KEY1 8
#define KEY2 4

//��ȭ ���� �ҷ�����
CDlg::CDlg()
{
	if(fp=fopen("data\\cloud.dlg","rb"))
	{
		//��� �� �б�
		fread(&snrs,sizeof(int),1,fp);
		//å���� �б�
		fread(&mark_no,sizeof(int),1,fp);
		fread(&bookmark,sizeof(int),mark_no,fp);
		//��系�� �б�
		snr=new char*[snrs];
		for(int snr_no=0; snr_no<snrs; snr_no++)
		{
			//���Ͽ��� �б�
			int text_size;
			fread(&text_size,sizeof(int),1,fp);
			snr[snr_no]=new char[text_size/2+1];

			char* buffer=new char[text_size+1];
			fread(buffer,text_size,1,fp);
			buffer[text_size]=NULL;

			//�ص�
			for(int i=0; i<text_size/2; i++)
			{
				char b1, b2;
				b1=buffer[i*2];
				b2=buffer[i*2+1];
				b1=get_char_num(b1)-KEY1;
				b2=get_char_num(b2)-30-KEY2;

				snr[snr_no][i]=b1*16+b2;
			}
			snr[snr_no][text_size/2]=NULL;
			
			delete[] buffer;
		}
	}
}

//��ȭ���� ����
CDlg::~CDlg()
{
	for(int i=0; i<snrs; i++)delete[] snr[i];
}

//��� �����ֱ�
#define BACK_MENU	0
#define BACK_RANK	2
#define BACK_END	3
#define BACK_ENDING	4

void CDlg::ShowBack(int back, int select)
{
	//���θ޴�
	if(back==BACK_MENU)
	{
		for(int j=0; j<8; j++)
			jdd->DrawPicture(SCREEN_BUFFER,"Black",0,j*20,NULL);
		//���� ���
		if(select==0)
		{
			char* quiz[]={"�ٶ��� Ǯ�� �ǰ�,","Ǯ�� ���� �ǰ�,","���� �ð谡 �ǰ�,","�ð�� ���� �ǰ�,","���� ���� �ǰ�,","���� ���ڰ� �ǰ�,","���ڴ� ���� �ǰ�,","���� ���� �ǰ�,","���� ���� �ǰ�,","���� (   )�� �ȴ�."};
			for(int i=0; i<10; i++)
			{
				if(i<m_sv.var[VAR_SOLUTION])jdd->DrawText(SCREEN_BUFFER,quiz[i],font12,20,15*i+5,JColor(255,255,0));
					else jdd->DrawText(SCREEN_BUFFER,quiz[i],font12,20,15*i+5,JColor(255,255,255));
			}

		}
		else if(select==1)
		{
			for(int i=0; i<m_sv.var[VAR_PARTY_NUM]; i++)
			{
				jdd->DrawPicture(SCREEN_BUFFER,StrAdd("cd","Face",m_sv.var[VAR_PARTY+i]),(i%5)*60,(i/5)*80,NULL);
			}
		}
		//��������
		else if(select==2)
		{
			jdd->DrawText(SCREEN_BUFFER,StrAdd("cd","������: ",m_sv.var[VAR_MONEY]),font12,0,0,JColor(255,255,255));
			jdd->DrawText(SCREEN_BUFFER,StrAdd("ccc","��ġ: ",cloud_name[m_sv.var[VAR_SPOT]]," ����"),font12,0,15,JColor(255,255,255));
			if(m_sv.sw[SW_GET_BALLOON])
			{
				jdd->DrawText(SCREEN_BUFFER,StrAdd("cd","ǳ����: ",m_sv.var[VAR_MODEL]+1),font12,0,30,JColor(255,255,255));
				jdd->DrawText(SCREEN_BUFFER,StrAdd("cd","����: ",m_sv.var[VAR_AIR]),font12,0,45,JColor(255,255,255));
			}
			if(m_sv.sw[SW_GET_BALLOON] && !m_sv.sw[SW_LENT_END])jdd->DrawText(SCREEN_BUFFER,StrAdd("cd","�Ӵ��: ",m_sv.var[VAR_LENT_FEE]),font12,0,60,JColor(255,255,255));
		}
		//����
		else if(select==4)
		{
			int iy=0;
			for(int i=0; i<40; i++)
			{
				if(m_sv.sw[SW_ITEM+i])
				{
					jdd->DrawText(SCREEN_BUFFER,item_name[i],font12,0,iy,JColor(255,255,255));
					iy+=15;
				}
			}
		}
		//���
		else if(select==5)
		{
			for(int j=0; j<8; j++)
				jdd->DrawPicture(SCREEN_BUFFER,"Black",0,j*20,NULL);
			for(int i=0; i<40; i++)
			{
				if(m_sv.var[VAR_EQUIP+i]>0)
				{
					int px, py;
					px=i%8*40;
					py=i/8*20;
					jdd->DrawPicture(SCREEN_BUFFER,StrAdd("cd","���",i),px,py,NULL);
					jdd->DrawText(SCREEN_BUFFER,StrAdd("d",m_sv.var[VAR_EQUIP+i]),font20,px+16,py,JColor(255,255,255));
					if(m_sv.var[VAR_EQUIP_LIST]==i)jdd->DrawText(SCREEN_BUFFER,"A",font20,px,py,JColor(255,0,0));
					if(m_sv.var[VAR_EQUIP_LIST+1]==i)jdd->DrawText(SCREEN_BUFFER,"S",font20,px,py,JColor(255,0,0));
					if(m_sv.var[VAR_EQUIP_LIST+2]==i)jdd->DrawText(SCREEN_BUFFER,"D",font20,px,py,JColor(255,0,0));
					if(m_sv.var[VAR_EQUIP_LIST+3]==i)jdd->DrawText(SCREEN_BUFFER,"F",font20,px,py,JColor(255,0,0));
				}
			}
		}
	}
	else if(back==BACK_RANK)
	{
		static int y;
		static int wait_moment;
		static bool to_down;
		int live=10-m_sv.var[VAR_DEATH];
		//���
		if(y<0)
		{
			to_down=true;
			y=0;
			wait_moment=100;
		}
		else if(y>(live*70)-160)
		{
			to_down=false;
			y=(live*70)-160;
			wait_moment=100;
		}

		//�� �Ʒ��� ��ũ��
		if(wait_moment<=0)
		{
			if(to_down)y++;
				else y--;
		}
		else
		{
			if(wait_moment>100)wait_moment=100;
			wait_moment--;
		}

		for(int j=0; j<8; j++)
			jdd->DrawPicture(SCREEN_BUFFER,"Black",0,j*20,NULL);
		for(int i=0; i<live; i++)
		{
			int py=i*70-y;
			if(py < -70 || py > 160)continue;
			jdd->DrawPicture(SCREEN_BUFFER,StrAdd("cd","Face",m_sv.var[VAR_RANK+i]),0,py,NULL);
			if(i==0)
			{
				if(m_party.IsIt(m_sv.var[VAR_RANK]))
					jdd->DrawText(SCREEN_BUFFER,StrAdd("cc","è�ǿ� ",fighter[m_sv.var[VAR_RANK]-1]),font20,70,py+30,JColor(255,255,0));
				else
					jdd->DrawText(SCREEN_BUFFER,StrAdd("cc","è�ǿ� ",fighter[m_sv.var[VAR_RANK]-1]),font20,70,py+30,JColor(255,255,255));

			}
			else{
				if(m_party.IsIt(m_sv.var[VAR_RANK+i]))
					jdd->DrawText(SCREEN_BUFFER,StrAdd("dcc",i,"�� ",fighter[m_sv.var[VAR_RANK+i]-1]),font20,70,py+30,JColor(255,255,0));
				else
					jdd->DrawText(SCREEN_BUFFER,StrAdd("dcc",i,"�� ",fighter[m_sv.var[VAR_RANK+i]-1]),font20,70,py+30,JColor(255,255,255));
			}

			int put_money=m_sv.var[VAR_MONEY];
			int px=276;
			while(put_money>=10)
			{
				put_money/=10;
				px-=11;
			}

			jdd->DrawText(SCREEN_BUFFER,StrAdd("dc",m_sv.var[VAR_MONEY],"ī��"),font12,px+1,1,JColor(0,0,0));
			jdd->DrawText(SCREEN_BUFFER,StrAdd("dc",m_sv.var[VAR_MONEY],"ī��"),font12,px,0,JColor(255,255,0));

		}
	}	
	//���������� �ش�
	else if(back==BACK_END)
	{
		for(int j=0; j<8; j++)
			jdd->DrawPicture(SCREEN_BUFFER,"Black",0,j*20,NULL);

		char put[9];
		for(int i=0; i<answer_max; i++)put[i]=answer[i];
		for(int i=answer_max; i<8; i++)put[i]='_';
		put[8]=NULL;
		jdd->DrawText(SCREEN_BUFFER,put,font20,0,140,JColor(255,255,255));
	}
	//����
	else if(back==BACK_ENDING)
	{
		static int x;
		static int wait_moment;
		static bool to_right;
		//���
		if(x<0)
		{
			to_right=true;
			x=0;
			wait_moment=200;
		}
		else if(x>640)
		{
			to_right=false;
			x=640;
			wait_moment=200;
		}

		//�� �Ʒ��� ��ũ��
		if(wait_moment<=0)
		{
			if(to_right)x++;
				else x--;
		}
		else
		{
			if(wait_moment>200)wait_moment=200;
			wait_moment--;
		}

		RECT src_rect;
		SetRect(&src_rect,x,0,x+320,160);
		jdd->DrawPicture(SCREEN_BUFFER,"Staff",0,0,&src_rect);
	}	
}

#define SNR_START 5
//��ȭ ���
int CDlg::TextPrint(char* content, int y, int back)
{
	int length=strlen(content);
	int face;
	char dlg_buffer[1024];
	strcpy(dlg_buffer,content);
	bool ani_end=false;

	//�� �˾Ƴ���
	char tmp[4] = {NULL, NULL, NULL, NULL};
	for(int i=0; i<2; i++)
		tmp[i]=dlg_buffer[i];
	face=atoi(tmp);

	//��ȭ�� ����ϱ� �˸°� ó��
	int sp=SNR_START, msp=SNR_START, space;	//ó���ǰ� �ִ� ����Ʈ, �������� ó���� ����Ʈ, space�� �ִ� ��
	while(sp<length)
	{
		if(dlg_buffer[sp]==' ')space=sp;
		if(dlg_buffer[sp]=='\\')
		{
			msp=sp;
			dlg_buffer[sp]='\n';
		}
		if(sp-msp>=30)
		{
			dlg_buffer[space]='\n';
			msp=space;
		}
		sp++;
	}

	RECT text, shadow;
	SetRect(&text,0,y,320,240);
	SetRect(&shadow,1,y+1,320,240);

	sp=SNR_START;
	int n_of_e=0, tp=0;	//�ٹٲ� Ƚ��, ����ϴ� �ܾ�
	while(1)
	{
		if(PeekMessage(&msg,NULL,0,0,PM_NOREMOVE))
		{
			if(!GetMessage(&msg,NULL,0,0)) break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//��ȭ���
		char text_buffer[256];
		if(!ani_end)
		{
			if(sp<length && n_of_e<4)
			{
				if(dlg_buffer[sp]=='\n')n_of_e++;
				text_buffer[tp]=dlg_buffer[sp];
				if(dlg_buffer[sp]&0x80)	//�ѱ��� ��� 2ĭ ����
				{
					sp++; tp++;
					text_buffer[tp]=dlg_buffer[sp];
				}
				text_buffer[tp+1]=NULL;
				//������ ����
				sp++; tp++;
			}
			else ani_end=true;
		}
		else
		{
			//��������
			if(_GetKeyState(VK_RETURN))
			{
				if(!enter)
				{
					enter=true;
					if(sp<length)
					{
						ani_end=false;
						n_of_e=0;
						tp=0;
					}
					else break;
				}
			}
			else enter=false;
		}
		//���,�� ���
		ShowBack(back);
		if(face>=0)
		{
			jdd->DrawPicture(SCREEN_BUFFER,"FaceSet",0,86,NULL);
			jdd->DrawPicture(SCREEN_BUFFER,StrAdd("cd","Face",face),2,88,NULL);
		}
		//��ȭ ���
		jdd->DrawPicture(SCREEN_BUFFER,"Dlg",0,160,NULL);
		jdd->DrawText(SCREEN_BUFFER,text_buffer,font20,&shadow,JColor(0,0,0));
		jdd->DrawText(SCREEN_BUFFER,text_buffer,font20,&text,JColor(255,255,255));

		Render();
		if(!ani_end && !_GetKeyState(VK_RETURN))Sleep(20);
	}

	//���� ��� �˾Ƴ���
	for(int i=0; i<3; i++)
		tmp[i]=dlg_buffer[i+2];
	tmp[3]=NULL;
	return atoi(tmp);
}

//��ȭó���� ����
void CDlg::TextSnr(int from, int back, bool erase)
{
	int command=0;
	from=bookmark[from];
	while(command!=999)
	{
		command=TextPrint(snr[from],160,back);
		from++;
	}
	if(erase)jdd->DrawPicture(SCREEN_BUFFER,"Dlg",0,160,NULL);
}

//���� ���
void CDlg::TextPut(char* content, int back)
{
	TextPrint(content,160,back);
	jdd->DrawPicture(SCREEN_BUFFER,"Dlg",0,160,NULL);
}

//������
int CDlg::TextSel(int n_dlg, int back, int start)
{
	strcpy(global_buffer,snr[bookmark[n_dlg]]);
	return TextSelect(160, back, start);
}

int CDlg::TextSel(char* input_dlg, int back, int start)
{
	strcpy(global_buffer,input_dlg);
	return TextSelect(160, back, start);
}

int CDlg::TextSelect(int y, int back, int start)
{
	char dlg_buffer[32][40];
	int length=strlen(global_buffer);

	//��ȭ�� ����ϱ� �˸°� ó��
	int n_of_e=0, select=start, bp=0;
	for(int i=0; i<length; i++)
	{
		if(global_buffer[i]=='\\')
		{
			dlg_buffer[n_of_e][bp]=NULL;
			n_of_e++;
			bp=0;
			continue;
		}
		if(global_buffer[i]==13)
		{
			continue;
		}

		dlg_buffer[n_of_e][bp]=global_buffer[i];
		bp++;
	}
	dlg_buffer[n_of_e][bp]=NULL;

	while(1)
	{
		if(PeekMessage(&msg,NULL,0,0,PM_NOREMOVE))
		{
			if(!GetMessage(&msg,NULL,0,0)) break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//������
		if(GetKeyDown())
		{
			select=Min(n_of_e,select+1);
		}
		else if(GetKeyUp())
		{
			select=Max(0,select-1);
		}
		if(GetKeyRight())
		{
			if(select+4<=n_of_e)select+=4;
		}
		else if(GetKeyLeft())
		{
			if(select-4>=0)select-=4;
		}

		if(_GetKeyState(VK_RETURN))
		{
			if(!enter)
			{
				enter=true;
				break;
			}
		}
		else enter=false;
		
		//������
		ShowBack(back,select);
		jdd->DrawPicture(SCREEN_BUFFER,"Dlg",0,160,NULL);

		int gap;
		if(n_of_e<4)gap=320;
			else if(n_of_e<8)gap=160;
			else if(n_of_e<16)gap=80;
			else gap=40;

		//���ù�
		int sx, sy;
		sx=select/4*gap;
		sy=y+(select%4)*20;
		RECT src_rect;
		SetRect(&src_rect,0,0,gap/2,20);
		jdd->DrawPicture(SCREEN_BUFFER,"SelectBar",sx,sy,&src_rect);
		SetRect(&src_rect,320-gap/2,0,320,20);
		jdd->DrawPicture(SCREEN_BUFFER,"SelectBar",sx+gap/2,sy,&src_rect);

		//��������
		for(int i=0; i<=n_of_e; i++)
		{
			jdd->DrawText(SCREEN_BUFFER,dlg_buffer[i],font20,(i/4)*gap+1,(i%4)*20+1+y,JColor(0,0,0));
			jdd->DrawText(SCREEN_BUFFER,dlg_buffer[i],font20,(i/4)*gap,(i%4)*20+y,JColor(255,255,255));
		}

		Render();
	}

	jdd->DrawPicture(SCREEN_BUFFER,"Dlg",0,160,NULL);
	return select;
}

CDlg m_dlg;

/////////////////////////////////
//��� ó�� Ŭ����
class CCommand
{
	int com_id[16];
	int command_max;
	int no, kind, back;
	char commands[256];

	int GetComID(int no);
	bool IsFull();

public:
	void AddComs(int n, ...);
	void AddCom(int id);
	int GetCommand();
	bool Empty();

	CCommand(int k, int max=32);
};

CCommand::CCommand(int k, int max)	//�ʱ�ȭ
{
	no=0;
	kind=k;
	back=-1;
	command_max=max;

	strcpy(commands,"");
	for(int i=0; i<5; i++)com_id[i]=-1;
}

void CCommand::AddComs(int n, ...)
{
	va_list ap;
	va_start(ap,n);
	for(int i=0; i<n; i++)AddCom(va_arg(ap,int));
	va_end(ap);
}

void CCommand::AddCom(int id)	//����� �� �ִ� ��� �߰�
{
	if(no>=command_max)return;	//��ɾ �� ��

	char* com_base[9]={"������.","����� ��´�.","������ ���.","ǳ���� ���.","������ �Ѵ�.","�踦 �����Ѵ�.","å�� �д´�.","�Ӵ�Ḧ ����.","�ش��� ���Ѵ�."};
	char* com_card[16]={"1","2","3","4","5","6","7","8","9","10","11","12","Ȧ��","¦��","6����","7�̻�"};
	char* com_fight[2]={"������.","������û"};
	char* com_challenge[5]={"���","1��� �� ����(1000ī��)","2��� �� ����(2000ī��)","3��� �� ����(3000ī��)","Ÿ��Ʋ ��ġ(10000ī��)"};
	char* com_manage[3]={"���","�ذ�","��ȭ�ϴ�"};
	char* com_book[8]={"������.","������ ����2(��)","�¾��� �ð�","���������� �� ���谡","������ ����2(��)","����","������ ����","������ ����2(��)"};
	char* com_gang[2]={"1000ī���� ����.","�ο��."};
	char* com_win[3]={"���� ������ ������ �δ�.","����� ��´�.","���� ��´�.(+1�� ī��)"};
	char* com_glass[2]={"5��ī���� ����.","������."};
	
	if(no>0)strcat(commands,"\\");
	switch(kind)
	{
		case 0: strcat(commands,com_base[id]); break;
		case 1: strcat(commands,com_card[id]); break;
		case 2: strcat(commands,com_fight[id]); back=BACK_RANK; break;
		case 3:	strcat(commands,fighter[id]); back=BACK_RANK; break;
		case 4:	strcat(commands,com_challenge[id]); back=BACK_RANK; break;
		case 5:	strcat(commands,com_manage[id]); break;
		case 6:	strcat(commands,com_book[id]); break;
		case 7:	strcat(commands,com_gang[id]); break;
		case 8: strcat(commands,com_win[id]); break;
		case 9: strcat(commands,com_glass[id]); break;
	}
	com_id[no]=id;
	no++;
}

int CCommand::GetComID(int no)
{
	if(no>=0 && no<command_max)return com_id[no];
		else return -1;
}

bool CCommand::IsFull()
{
	if(no>=command_max)return true;
		else return false;
}

int CCommand::GetCommand()
{
	return GetComID(m_dlg.TextSel(commands,back));
}

bool CCommand::Empty()
{
	if(no==0)return true;
		else return false;
}

///////////////////////////////////////////////////////////////////////////////
//���� ��ü
class CGame
{
	void Management();
	void Equip();
	void Save(int file_no);
public:
	bool Load(int file_no);
	void Day(int d);
	void Time(int t);
	int Menu();
	void Shop(int goods_number, ...);
	void BalloonShop();
};

void CGame::Save(int file_no)
{
	if(fp=fopen(StrAdd("cdc","Save\\cloud",file_no,".sav"),"wb"))
	{
		//���� ǥ��
		fwrite("ū����10",sizeof(char),8,fp);
		//���� ����
		fwrite(&m_sv.var[VAR_SPOT],sizeof(int),1,fp);
		fwrite(&m_sv.var[VAR_TIME],sizeof(int),4,fp);
		//����
		fwrite(&m_sv,sizeof(CSW_VAR),1,fp);
		fclose(fp);
	}
}

bool CGame::Load(int file_no)
{
	if(fp=fopen(StrAdd("cdc","Save\\cloud",file_no,".sav"),"rb"))
	{
		fseek(fp,28l,SEEK_SET);
		fread(&m_sv,sizeof(CSW_VAR),1,fp);
		fclose(fp);
	}
	else return false;

	return true;
}

void CGame::Day(int d)
{
	m_sv.var[VAR_TIME+2]+=d;
	while(m_sv.var[VAR_TIME+2]>30)
	{
		m_sv.var[VAR_TIME+2]-=30;
		m_sv.var[VAR_TIME+1]++;
		if(!m_sv.sw[SW_LENT_END])m_sv.var[VAR_LENT_FEE]+=1000;
		if(m_party.IsIt(m_sv.var[VAR_RANK]))m_sv.var[VAR_MONEY]+=10000;
	}
	while(m_sv.var[VAR_TIME+1]>12)
	{
		m_sv.var[VAR_TIME+1]-=12;
		m_sv.var[VAR_TIME]++;
	}
}

void CGame::Time(int t)
{
	m_sv.var[VAR_TIME+3]+=t;
	while(m_sv.var[VAR_TIME+3]>=24)
	{
		m_sv.var[VAR_TIME+3]-=24;
		Day(1);
	}
}

int CGame::Menu()
{
	int select=0;
	while(1)
	{
		select=m_dlg.TextSel(20,BACK_MENU,select);
		//����
		if(select==1)
		{
			Management();
		}
		//����
		else if(select==3)
		{
			char savefiles[256];
			int load_point=0;
			
			//���� �б�
			strcpy(savefiles,"");
			for(int i=1; i<=FILE_MAX; i++)
			{					
				if(fp=fopen(StrAdd("cdc","Save\\cloud",i,".sav"),"rb"))
				{
					int save_spot;
					int save_time[4];
					fseek(fp,8,SEEK_SET);
					fread(&save_spot,sizeof(int),1,fp);
					fread(&save_time,sizeof(int),4,fp);
					strcat(savefiles,StrAdd("dcdcdcdc",save_time[0],"��",save_time[1],"��",save_time[2],"��",save_time[3],"�� "));
					strcat(savefiles,StrAdd("cc",cloud_name[save_spot],"����"));
					fclose(fp);
				}
				else strcat(savefiles,"��� ����");
				strcat(savefiles,"\\");
			}
			strcat(savefiles,"���");

			int no=m_dlg.TextSel(savefiles);
			if(no<3)Save(no+1);
		}
		//���
		else if(select==5)
		{
			Equip();
		}
		//���� ������
		else if(select==7)
		{
			if(m_dlg.TextSel(13)==3)return 999;
		}
		//����
		else if(select==6)break;
	}

	return 0;
}

//����
void CGame::Shop(int goods_number, ...)
{
	//�ʱ⼳��
	int goods[10];
	int goods_num[10];
	int equip_price[40];
	char* equip_name[40];
	//�̸��� ��
	equip_name[0]="�豸��";
	equip_price[0]=10;
	equip_name[1]="�ձ���";
	equip_price[1]=30;
	equip_name[2]="3��ź";
	equip_price[2]=20;
	equip_name[3]="�Ͼ���";
	equip_price[3]=300;
	equip_name[4]="���๰";
	equip_price[4]=500;
	equip_name[5]="16��";
	equip_price[5]=250;
	equip_name[6]="����";
	equip_price[6]=100;
	equip_name[20]="����";
	equip_price[20]=500;
	equip_name[21]="����";
	equip_price[21]=2000;

	//�Ĵ� ��ǰ �ҷ�����
	va_list ap;
	va_start(ap,goods_number);
	for(int i=0; i<goods_number; i++)
	{
		goods[i]=va_arg(ap,int);
		goods_num[i]=0;
	}
	va_end(ap);

	int select=0;
	while(1)
	{
		if(PeekMessage(&msg,NULL,0,0,PM_NOREMOVE))
		{
			if(!GetMessage(&msg,NULL,0,0)) break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		for(int j=0; j<8; j++)
			jdd->DrawPicture(SCREEN_BUFFER,"Black",0,j*20,NULL);

		//����
		if(GetKeyUp())select=Max(0,select-1);
			else if(GetKeyDown())select=Min(goods_number+1,select+1);
		//���� ����
		if(select<goods_number)
		{
			if(_GetKeyState(VK_LEFT))goods_num[select]=Max(0,goods_num[select]-1);
				else if(_GetKeyState(VK_RIGHT))goods_num[select]=Min((99-m_sv.var[VAR_EQUIP+goods[select]]),goods_num[select]+1);
		}
		//��� �����ֱ�
		int total=0;
		int i;
		
		for(i=0; i<goods_number; i++)
		{
			jdd->DrawPicture(SCREEN_BUFFER,StrAdd("cd","���",goods[i]),0,i*20+1,NULL);
			jdd->DrawText(SCREEN_BUFFER,equip_name[goods[i]],font20,20,i*20,JColor(255,255,255));
			jdd->DrawText(SCREEN_BUFFER,StrAdd("dc",equip_price[goods[i]],"ī��"),font20,120,i*20,JColor(255,255,255));
			jdd->DrawText(SCREEN_BUFFER,StrAdd("dc",goods_num[i],"��"),font20,240,i*20,JColor(255,255,255));

			total+=equip_price[goods[i]]*goods_num[i];
		}
		//����ǥ��
		if(select<goods_number)
		{
			jdd->DrawPicture(SCREEN_BUFFER,"SelectBar2",0,select*20,NULL);
		}
		else
		{
			RECT tmp_rect;
			SetRect(&tmp_rect,0,0,40,20);
			jdd->DrawPicture(SCREEN_BUFFER,"SelectBar2",240,select*20,&tmp_rect);
			SetRect(&tmp_rect,280,0,320,20);
			jdd->DrawPicture(SCREEN_BUFFER,"SelectBar2",280,select*20,&tmp_rect);
		}

		//���� ���
		jdd->DrawText(SCREEN_BUFFER,StrAdd("cd","�Ѿ�: ",total),font20,40,i*20,JColor(255,255,255));
		jdd->DrawText(SCREEN_BUFFER,StrAdd("cd","����: ",m_sv.var[VAR_MONEY]),font20,40,(i+1)*20,JColor(255,255,255));
		if(total<=m_sv.var[VAR_MONEY])
			jdd->DrawText(SCREEN_BUFFER,"����",font20,260,i*20,JColor(255,255,255));
		else
			jdd->DrawText(SCREEN_BUFFER,"����",font20,260,i*20,JColor(64,64,64));
		jdd->DrawText(SCREEN_BUFFER,"���",font20,260,(i+1)*20,JColor(255,255,255));

		Render();

		//����
		if(_GetKeyState(VK_RETURN))
		{
			if(!enter)
			{
				enter=true;
				//����
				if(select==goods_number && m_sv.var[VAR_MONEY]>=total)
				{
					_Play(10);
					m_sv.var[VAR_MONEY]-=total;
					for(int i=0; i<goods_number; i++)
						m_sv.var[VAR_EQUIP+goods[i]]+=goods_num[i];
					break;
				}
				//���
				else if(select==goods_number+1)break;
			}
		}
		else enter=false;

		//����ϰ� ������
		if(_GetKeyState(VK_ESCAPE))
		{
			if(!esc)
			{
				esc=true;
				break;
			}
		}
		else esc=false;
	}
}

//ǳ������
void CGame::BalloonShop()
{
	//�ʱ⼳��
	int goods[5];
	int balloon_price[5];
	int power[5], move[5], speed[5];
	char* balloon_name[40];
	int have_balloon;
	//�̸��� ��, ����
	balloon_name[0]="�α��� ǳ��";
	balloon_price[0]=100000;
	power[0]=1;
	move[0]=2;
	speed[0]=8;
	
	balloon_name[1]="������ ǳ��";
	balloon_price[1]=200000;
	power[1]=1;
	move[1]=4;
	speed[1]=10;
	
	balloon_name[2]="���� ǳ��";
	balloon_price[2]=400000;
	power[2]=2;
	move[2]=5;
	speed[2]=12;
	
	balloon_name[3]="�ʰ�� ǳ��";
	balloon_price[3]=650000;
	power[3]=1;
	move[3]=7;
	speed[3]=20;

	balloon_name[4]="�尩 ǳ��";
	balloon_price[4]=500000;
	power[4]=4;
	move[4]=3;
	speed[4]=11;

	if(m_sv.sw[SW_LENT_END])have_balloon=balloon_price[m_sv.var[VAR_MODEL]]/2;
		else have_balloon=-m_sv.var[VAR_LENT_FEE];
	for(int i=0; i<5; i++)
	{
		balloon_price[i]-=have_balloon;
		if(m_sv.sw[SW_ITEM+3])balloon_price[i]=balloon_price[i]*9/10;
	}

	//ǳ�� ����
	int max=0;
	for(int i=0; i<5; i++)
	{
		if(m_sv.var[VAR_MODEL]==i)
		{
			if(i==0 && !m_sv.sw[SW_LENT_END])
			{
				goods[max]=i;
				max++;
			}
		}
		else
		{
			goods[max]=i;
			max++;
		}
	}

	int select=0;
	while(1)
	{
		if(PeekMessage(&msg,NULL,0,0,PM_NOREMOVE))
		{
			if(!GetMessage(&msg,NULL,0,0)) break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		for(int j=0; j<8; j++)
			jdd->DrawPicture(SCREEN_BUFFER,"Black",0,j*20,NULL);

		//ǳ�� ����
		if(GetKeyUp())select=Max(0,select-1);
			else if(GetKeyDown())select=Min(max-1,select+1);
		for(int i=0; i<max; i++)
		{
			jdd->DrawText(SCREEN_BUFFER,balloon_name[goods[i]],font20,20,i*20,JColor(255,255,255));
			if(m_sv.var[VAR_MONEY]>=balloon_price[goods[i]])jdd->DrawText(SCREEN_BUFFER,StrAdd("dc",balloon_price[i],"ī��"),font20,160,i*20,JColor(255,255,255));
				else jdd->DrawText(SCREEN_BUFFER,StrAdd("dc",balloon_price[goods[i]],"ī��"),font20,160,i*20,JColor(255,0,0));
		}
		//����ǥ��
		jdd->DrawPicture(SCREEN_BUFFER,"SelectBar2",0,select*20,NULL);
		jdd->DrawPicture(SCREEN_BUFFER,StrAdd("cd","Ship",goods[select]+1),0,100,NULL);
		jdd->DrawText(SCREEN_BUFFER,StrAdd("cd","��: ",power[goods[select]]),font20,160,100,JColor(255,255,0));
		jdd->DrawText(SCREEN_BUFFER,StrAdd("cd","�̵���: ",move[goods[select]]),font20,160,120,JColor(255,255,0));
		jdd->DrawText(SCREEN_BUFFER,StrAdd("cd","�ִ�ӷ�: ",speed[goods[select]]),font20,160,140,JColor(255,255,0));

		Render();

		//����
		if(_GetKeyState(VK_RETURN))
		{
			if(!enter)
			{
				enter=true;
				//����
				if(m_sv.var[VAR_MONEY]>=balloon_price[goods[select]])
				{
					m_sv.var[VAR_MONEY]-=balloon_price[goods[select]];
					m_sv.var[VAR_MODEL]=goods[select];
					if(!m_sv.sw[SW_LENT_END])
					{
						m_sv.sw[SW_LENT_END]=true;
						m_sv.var[VAR_LENT_FEE]=0;
					}
					m_sv.var[VAR_AIR]=1000;
					if(m_sv.sw[SW_ITEM+3])m_sv.sw[SW_ITEM+3]=false;
					break;
				}
			}
		}
		else enter=false;

		//����ϰ� ������
		if(_GetKeyState(VK_ESCAPE))
		{
			if(!esc)
			{
				esc=true;
				break;
			}
		}
		else esc=false;
	}
}

void CGame::Equip()
{
	int select=0;
	while(!_GetKeyState(VK_ESCAPE))
	{
		if(PeekMessage(&msg,NULL,0,0,PM_NOREMOVE))
		{
			if(!GetMessage(&msg,NULL,0,0)) break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		for(int j=0; j<8; j++)
			jdd->DrawPicture(SCREEN_BUFFER,"Black",0,j*20,NULL);

		//����
		if(GetKeyUp())
		{
			if(select>=8)select-=8;
		}
		else if(GetKeyDown())
		{
			if(select<32)select+=8;
		}

		if(m_sv.var[VAR_EQUIP+select]>0)
		{
			if(_GetKeyState('A'))
			{
				m_sv.var[VAR_EQUIP_LIST]=select;
			}
			else if(_GetKeyState('S'))
			{
				m_sv.var[VAR_EQUIP_LIST+1]=select;
			}
			else if(_GetKeyState('D'))
			{
				m_sv.var[VAR_EQUIP_LIST+2]=select;
			}
			else if(_GetKeyState('F'))
			{
				m_sv.var[VAR_EQUIP_LIST+3]=select;
			}
		}

		if(GetKeyLeft())select=Max(0,select-1);
			else if(GetKeyRight())select=Min(39,select+1);
		//��� �����ֱ�
		for(int i=0; i<40; i++)
		{
			if(m_sv.var[VAR_EQUIP+i]>0)
			{
				int px, py;
				px=i%8*40;
				py=i/8*20;
				jdd->DrawPicture(SCREEN_BUFFER,StrAdd("cd","���",i),px,py,NULL);
				jdd->DrawText(SCREEN_BUFFER,StrAdd("d",m_sv.var[VAR_EQUIP+i]),font20,px+16,py,JColor(255,255,255));
				if(m_sv.var[VAR_EQUIP_LIST]==i)jdd->DrawText(SCREEN_BUFFER,"A",font20,px,py,JColor(255,0,0));
				if(m_sv.var[VAR_EQUIP_LIST+1]==i)jdd->DrawText(SCREEN_BUFFER,"S",font20,px,py,JColor(255,0,0));
				if(m_sv.var[VAR_EQUIP_LIST+2]==i)jdd->DrawText(SCREEN_BUFFER,"D",font20,px,py,JColor(255,0,0));
				if(m_sv.var[VAR_EQUIP_LIST+3]==i)jdd->DrawText(SCREEN_BUFFER,"F",font20,px,py,JColor(255,0,0));
			}
		}
		//����ǥ��
		int sx, sy;
		sx=select%8*40;
		sy=select/8*20;

		RECT tmp_rect;
		SetRect(&tmp_rect,0,0,20,20);
		jdd->DrawPicture(SCREEN_BUFFER,"SelectBar2",sx,sy,&tmp_rect);
		SetRect(&tmp_rect,300,0,320,20);
		jdd->DrawPicture(SCREEN_BUFFER,"SelectBar2",sx+20,sy,&tmp_rect);

		Render();

		//��� ó�� �̺�Ʈ
		if(!m_sv.sw[SW_EQUIP_EXAM])
		{
			m_dlg.TextSnr(58);
			m_sv.sw[SW_EQUIP_EXAM]=true;
		}
	}
}

void CGame::Management()
{
	int select=0;
	while(!_GetKeyState(VK_ESCAPE))
	{
		if(PeekMessage(&msg,NULL,0,0,PM_NOREMOVE))
		{
			if(!GetMessage(&msg,NULL,0,0)) break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//����
		if(GetKeyUp())
		{
			if(select>=5)select-=5;
		}
		else if(GetKeyDown())
		{
			if(select<5 && m_sv.var[VAR_PARTY_NUM]>(5+select))select+=5;
		}

		//����
		if(_GetKeyState(VK_RETURN))
		{
			if(!enter)
			{
				enter=true;
				//��ȭ, �ذ�
				if(m_sv.var[VAR_PARTY_NUM]>select)
				{
					int target=m_sv.var[VAR_PARTY+select];
					CCommand m_talk(5);
					//�ϴ������� ��ȭ
					if(target==3 || target==4 || target==8 || target==19 || target==24 || target==29)
					{
						if(TRIBE_SKY)m_talk.AddCom(2);
					}
					//���������� ��ȭ
					else if(target==6 || target==14 || target==15 || target==23 || target==31)
					{
						if(TRIBE_LIGHTNING)m_talk.AddCom(2);
					}
					else m_talk.AddCom(2);

					if(target!=20 && target!=12 && target!=18)m_talk.AddCom(1);
					m_talk.AddCom(0);

					int com=m_talk.GetCommand();
					if(com==1)m_party.Delete(target);	//�ذ�
					else if(com==2)m_dlg.TextSnr(87+target);	//��ȭ
				}
			}
		}
		else enter=false;

		if(GetKeyLeft())select=Max(0,select-1);
			else if(GetKeyRight() &&m_sv.var[VAR_PARTY_NUM]>select+1)select=Min(9,select+1);

		//ȭ��
		for(int j=0; j<8; j++)
			jdd->DrawPicture(SCREEN_BUFFER,"Black",0,j*20,NULL);
		//���� �����ֱ�
		for(int i=0; i<m_sv.var[VAR_PARTY_NUM]; i++)
		{
			jdd->DrawPicture(SCREEN_BUFFER,StrAdd("cd","Face",m_sv.var[VAR_PARTY+i]),(i%5)*60,(i/5)*80,NULL);
		}
		//����ǥ��
		int sx, sy;
		sx=select%5*60+20;
		sy=select/5*80+60;
		jdd->DrawPicture(SCREEN_BUFFER,"Cursor",sx,sy,NULL);

		Render();
	}
}

CGame m_game;

//////////////////////////////////////////////////////////////////////////////
//���� Ŭ����

//ĳ���� ������
class CSprData
{
public:
	char* name;

	int power;
	int speed;
	int hpmax;
	int x_size, y_size;

	void SetData(char* vname, int vpower, int vspeed, int vhpmax);
};

void CSprData::SetData(char* vname, int vpower, int vspeed, int vhpmax)
{
	name=vname;
	power=vpower;
	speed=vspeed;
	hpmax=vhpmax;

	JPictureInfo pi;
	jdd->GetPictureInfo(name,&pi);
	x_size=pi.GetWidth();
	y_size=pi.GetHeight();
}

CSprData spr_data[100];

#define DATA_NAME(x)	spr_data[x].name
#define DATA_XS(x)		spr_data[x].x_size
#define DATA_YS(x)		spr_data[x].y_size
#define DATA_POW(x)		spr_data[x].power
#define DATA_SPD(x)		spr_data[x].speed
#define DATA_HPMAX(x)	spr_data[x].hpmax

int key_a, key_s, key_d, key_f;

//��������Ʈ Ŭ����
class CSpr
{
	int tempo;
	int ani_tempo;
	int angle;		//�̻����� ����
	int t1, t2;		//�ڱ⸸�� Ư�� ����
	
	bool Goto(int tx, int ty);
	void Use(int weapon);

public:
	int type;
	int x, y;
	int hp;
	bool life;

	void SetSpr(int vx, int vy, int vtype);
	void SetMissile(int vx, int vy, int vangle, int vtype);
	void SetGradient(int x2, int y2);
	int Control();
	int Move();
	void Show();
	void Hurt(int damage);
};

bool CSpr::Goto(int tx, int ty)
{
	if(x<tx)x=Min(x+DATA_SPD(type),tx);
		else if(x>tx)x=Max(x-DATA_SPD(type),tx);
	if(y<ty)y=Min(y+DATA_SPD(type),ty);
		else if(y>ty)y=Max(y-DATA_SPD(type),ty);

	if(x==tx && y==ty)return true;
		else return false;
}

void CSpr::SetSpr(int vx, int vy, int vtype)
{
	//�ʱ�ȭ
	life=true;
	x=vx;
	y=vy;
	type=vtype;
	if(type==0)hp=m_sv.var[VAR_AIR];
		else hp=DATA_HPMAX(type);

	tempo=0;
	ani_tempo=0;

	if(type==6 || type==9 || type==13 || type==15)	//�Դ��� ��ġ
	{
		t1=(rand()%(200-DATA_XS(type)))+120;
		t2=(rand()%(160-DATA_YS(type)));
	}
	else if(type==14)	//�ղ�
	{
		t1=0;
	}
}

void CSpr::SetMissile(int vx, int vy, int vangle, int vtype)
{
	//�ʱ�ȭ
	life=true;
	x=vx;
	y=vy;
	angle=vangle;
	type=vtype;

	hp=DATA_HPMAX(type);
	tempo=0;
	ani_tempo=0;
}

int CSpr::Move()
{
	int command=-1;

	tempo++;
	if(type<5)	//���༱1
	{
		//�¿� ��鸲
		int shake=Max(0,m_sv.var[VAR_BANUL_SPEED]-m_sv.var[VAR_ABIL_PILOT]);
		shake=(rand()%(shake*2+1))-shake;
		x=Max(0,Min(320-DATA_XS(type),x+=shake));
		//���� ��鸲
		shake=Max(0,m_sv.var[VAR_BANUL_SPEED]-m_sv.var[VAR_ABIL_PILOT]);
		shake=(rand()%(shake*2+1))-shake;
		y=Max(0,Min(180-DATA_YS(type),y+=shake));
	}
	else if(type==5 || type==7)	//��, ����
	{
		x-=(DATA_SPD(type)+(m_sv.var[VAR_BANUL_SPEED]/2));
	}
	else if(type==6)	//����
	{
		Goto(t1,t2);
		if(tempo>=20)
		{
			command=50;
			tempo=0;
		}
	}
	else if(type==8)	//���� ��
	{
		x-=(DATA_SPD(type)+(m_sv.var[VAR_BANUL_SPEED]/2));
		if(tempo>=20)
		{
			command=52;
			tempo=0;
		}
	}
	else if(type==9)	//��������
	{
		Goto(t1,t2);
		if(tempo>=50)
		{
			command=51;
			tempo=0;
		}
	}
	else if(type==10 || type==11)	//�Ĺ�
	{
		Goto(x,180-DATA_YS(type));
		int attack;
		if(type==10)attack=50;
			else attack=20;
		if(tempo>=attack)
		{
			command=53;
			tempo=0;
		}
	}
	else if(type==12)	//��
	{
		y--;
		x+=(rand()%(DATA_SPD(type)*2+1))-DATA_SPD(type);
	}
	else if(type==13)	//�մ���
	{
		if(Goto(t1,t2))
		{
			t1=(rand()%(200-DATA_XS(type)))+120;
			t2=(rand()%(160-DATA_YS(type)));
		}

		//�̻��� ��� �߻�
		command=tempo;
		if(tempo>15)tempo=0;
	}
	else if(type==14)	//�ղ�
	{
		Goto(t1,180-DATA_YS(type));
		if(x==0)t1=320-DATA_XS(type);
			else if(x==320-DATA_XS(type))t1=0;
		if(tempo>=10)
		{
			command=53;
			tempo=0;
		}
	}
	else if(type==15)	//Ž���ڿ��� ���
	{
		if(Goto(t1,t2))
		{
			t1=(rand()%(320-DATA_XS(type)));
			t2=(rand()%(160-DATA_YS(type)));
		}

		//�ձ��� �߻�
		if(tempo>15)
		{
			command=54;
			tempo=0;
		}
	}		
	else if(type==57)	//��
	{
		if(hp>=0)
		{
			hp--;
			if(hp<0)life=false;
		}
	}

	//�̻���
	else if(type>=50)
	{
		if(angle<0)angle+=16;
			else if(angle>=16)angle-=16;

		int x_percent, y_percent;
		if(angle==0){ x_percent=0; y_percent=-4; }
			else if(angle==1){ x_percent=1; y_percent=-3; }
			else if(angle==2){ x_percent=2; y_percent=-2; }
			else if(angle==3){ x_percent=3; y_percent=-1; }
			else if(angle==4){ x_percent=4; y_percent=0; }
			else if(angle==5){ x_percent=3; y_percent=1; }
			else if(angle==6){ x_percent=2; y_percent=2; }
			else if(angle==7){ x_percent=1; y_percent=3; }
			else if(angle==8){ x_percent=0; y_percent=4; }
			else if(angle==9){ x_percent=-1; y_percent=3; }
			else if(angle==10){ x_percent=-2; y_percent=2; }
			else if(angle==11){ x_percent=-3; y_percent=1; }
			else if(angle==12){ x_percent=-4; y_percent=0; }
			else if(angle==13){ x_percent=-3; y_percent=-1; }
			else if(angle==14){ x_percent=-2; y_percent=-2; }
			else if(angle==15){ x_percent=-1; y_percent=-3; }

		if((x_percent%4)==0)x+=DATA_SPD(type)*x_percent/4;
			else if(angle<8)x+=DATA_SPD(type)*x_percent/4+1;
			else x+=DATA_SPD(type)*x_percent/4-1;
		if((y_percent%4)==0)y+=DATA_SPD(type)*y_percent/4;
			else if(angle<4 || angle>=12)y+=DATA_SPD(type)*y_percent/4-1;
			else y+=DATA_SPD(type)*y_percent/4+1;
		//������ �� �ִ� �ð�(-1: ��� ����)
		if(hp>=0)
		{
			hp--;
			if(hp<0)life=false;
		}

		//�豸��
		if(type==52)y+=tempo/20;
	}		

	//ȭ������� ����
	if(x<-DATA_XS(type) || x>320 || y<-DATA_YS(type) || y>180)life=false;

	return command;
}

void CSpr::Use(int weapon)
{
	bool minus=true;
	//������� ���� ����
	if(m_party.IsIt(28))
	{
		if(weapon==VAR_EQUIP || weapon==VAR_EQUIP+1)minus=false;
	}
	//������ 3��ź ����
	if(m_party.IsIt(29) && weapon==VAR_EQUIP+2)minus=false;
	//������ �Ͼ��� ����
	if(m_party.IsIt(31) && weapon==VAR_EQUIP+3)minus=false;
		
	if(minus)m_sv.var[weapon]--;
}

int CSpr::Control()
{
	int command=-1;
	//�̵�
	if(_GetKeyState(VK_UP))y=Max(0,y-DATA_SPD(type));
		else if(_GetKeyState(VK_DOWN))y=Min(180-DATA_YS(type),y+DATA_SPD(type));
	if(_GetKeyState(VK_LEFT))x=Max(0,x-DATA_SPD(type));
		else if(_GetKeyState(VK_RIGHT))x=Min(320-DATA_XS(type),x+DATA_SPD(type));
	//�극��ũ
	if(_GetKeyState(VK_SPACE))
	{
		if(!key_space)
		{
			key_space=true;
			command=999;
		}
	}
	else key_space=false;

	//����
	//A�� ������ ����
	if(_GetKeyState('A') && m_sv.var[VAR_EQUIP+m_sv.var[VAR_EQUIP_LIST]]>0)
	{
		if(key_a==0)
		{
			command=m_sv.var[VAR_EQUIP_LIST];
			Use(VAR_EQUIP+m_sv.var[VAR_EQUIP_LIST]);
			key_a=30;
		}
		else key_a--;
	}
	else key_a=0;
	//S�� ������ ����
	if(_GetKeyState('S') && m_sv.var[VAR_EQUIP+m_sv.var[VAR_EQUIP_LIST+1]]>0)
	{
		if(key_s==0)
		{
			command=m_sv.var[VAR_EQUIP_LIST+1];
			Use(VAR_EQUIP+m_sv.var[VAR_EQUIP_LIST+1]);
			key_s=30;
		}
		else key_s--;
	}
	else key_s=0;
	//D�� ������ ����
	if(_GetKeyState('D') && m_sv.var[VAR_EQUIP+m_sv.var[VAR_EQUIP_LIST+2]]>0)
	{
		if(key_d==0)
		{
			command=m_sv.var[VAR_EQUIP_LIST+2];
			Use(VAR_EQUIP+m_sv.var[VAR_EQUIP_LIST+2]);
			key_d=30;
		}
		else key_d--;
	}
	else key_d=0;
	//F�� ������ ����
	if(_GetKeyState('F') && m_sv.var[VAR_EQUIP+m_sv.var[VAR_EQUIP_LIST+3]]>0)
	{
		if(key_f==0)
		{
			command=m_sv.var[VAR_EQUIP_LIST+3];
			Use(VAR_EQUIP+m_sv.var[VAR_EQUIP_LIST+3]);
			key_f=30;
		}
		else key_f--;
	}
	else key_f=0;

	return command;
}

void CSpr::Show()
{
	jdd->DrawPicture(SCREEN_BUFFER,DATA_NAME(type),x,y,NULL);
}

void CSpr::Hurt(int damage)
{
	hp=Max(0,hp-damage);
	if(hp==0)
	{
		if(type<5)_Play(21);
		else if(type==13 || type==14)
		{
			m_sv.var[VAR_PRIZE]+=30000;
			_Play(23);
		}
		else if(type!=12 && type<50)_Play(22);
		life=false;
	}
}

//��������Ʈ, �̻��� ����
#define SHIELD_SP			1		//���� ID
#define ENEMY_SP			20		//�� ID�� ����
#define MISSILE_SP			80		//�Ʊ� �̻��� ID�� ����
#define ENEMY_MISSILE_SP	120		//�� �̻��� ID�� ����
#define SPR_MAX				200		//�̻��� ���� �Ѱ�

#define CREATE_SHIP				0
#define CREATE_SPR				1
#define CREATE_ARMY_MISSILE		2
#define CREATE_ENEMY_MISSILE	3
#define CREATE_SHIELD			4

//���� ó��
class CShooting
{
	CSpr spr[SPR_MAX];
	int Find(int st, int ed, bool life);
	void Create(int command, int v1, int v2, int v3);
	int stage_length;
	int time, second;
	int count, banul, stage_no, banul_speed;

	bool Crush(int id1, int id2);
	int CenterX(int spr_no, int msl_type);
	int CenterY(int spr_no, int msl_type);

public:
	CShooting(int sn);
	int Play();
};

CShooting::CShooting(int sn)
{
	stage_no=sn;

	//��������Ʈ �ʱ�ȭ
	for(int i=0; i<SPR_MAX; i++)spr[i].life=false;
	Create(CREATE_SHIP,10,60,m_sv.var[VAR_MODEL]);
	
	//�ٶ��� ����
	switch(stage_no)
	{
		case 0: stage_length=150; break;
		case 1: stage_length=200; break;
		case 2: stage_length=100; break;
		case 3: stage_length=100; break;
		case 4: stage_length=50; break;
		case 5: stage_length=300; break;
		case 6: stage_length=250; break;
		case 7: stage_length=225; break;
		case 8: stage_length=250; break;
		case 9: stage_length=200; break;

		//����� ��
		case 10: stage_length=0;
			Create(CREATE_SPR,320,20,15);
			break;
	}

	//�ʱ�ȭ
	time=second=0;
	count=banul=0;
	banul_speed=1;
	m_sv.var[VAR_PRIZE]=0;

	//������ �ɷ� �˻�
	m_sv.var[VAR_ABIL_PILOT]=0;
	int control[]={2,5,4,3,5,6};
	for(int i=19; i<=24; i++)
		if(m_party.IsIt(i))m_sv.var[VAR_ABIL_PILOT]+=control[i-20];
}

//��밡���� ��ȣ �˻�
int CShooting::Find(int st, int ed, bool life)
{
	for(int i=st; i<ed; i++)
		if(spr[i].life==life)return i;
	return -1;
}

//���� ����
void CShooting::Create(int command, int v1, int v2, int v3)
{
	//v1 = X��ǥ, v2 = Y��ǥ, v3 = Ÿ��
	if(command==CREATE_SHIP)
	{
		spr[0].SetSpr(v1,v2,v3);
	}
	if(command==CREATE_SPR)
	{
		int id=Find(ENEMY_SP,MISSILE_SP,false);
		if(id>=0)spr[id].SetSpr(v1,v2,v3);
	}
	//v1 = ����, v2 = ����, v3 = Ÿ��
	else if(command==CREATE_ARMY_MISSILE)
	{		
		int id=Find(MISSILE_SP,ENEMY_MISSILE_SP,false);
		if(id>=0)
		{
			if(v3==56)spr[id].SetMissile(spr[0].x,-DATA_YS(v3),v2,v3);
				else spr[id].SetMissile(CenterX(v1,v3),CenterY(v1,v3),v2,v3);
		}
	}
	else if(command==CREATE_ENEMY_MISSILE)
	{
		int id=Find(ENEMY_MISSILE_SP,SPR_MAX,false);
		if(id>=0)spr[id].SetMissile(CenterX(v1,v3),CenterY(v1,v3),v2,v3);
	}
	//��
	else if(command==CREATE_SHIELD)
	{
		int id=Find(SHIELD_SP,ENEMY_SP,false);
		if(id>=0)spr[id].SetSpr(spr[0].x+60,CenterY(v1,v3),v3);
	}
}

bool CShooting::Crush(int id1, int id2)
{
	if(spr[id1].x+DATA_XS(spr[id1].type) > spr[id2].x &&
		spr[id1].x <=spr[id2].x+DATA_XS(spr[id2].type) &&
		spr[id1].y+DATA_YS(spr[id1].type) > spr[id2].y &&
		spr[id1].y <=spr[id2].y+DATA_YS(spr[id2].type))return true;
	else return false;
}

int CShooting::CenterX(int spr_no, int msl_type)
{
	return spr[spr_no].x+(DATA_XS(spr[spr_no].type)/2)-(DATA_XS(msl_type)/2);
}

int CShooting::CenterY(int spr_no, int msl_type)
{
	return spr[spr_no].y+(DATA_YS(spr[spr_no].type)/2)-(DATA_YS(msl_type)/2);
}

int CShooting::Play()
{
	bool clear=false;
	_MidiPlay(bgm[9]);

	int max;
	switch(m_sv.var[VAR_MODEL])
	{
		case 0: max=8; break;
		case 1: max=10; break;
		case 2: max=12; break;
		case 3: max=20; break;
		case 4: max=11; break;
	}

	bool fly_end=false;
	while(/*(!_GetKeyState('Q') || !_GetKeyState('L')) &&*/ !fly_end && spr[0].life)
	{
		if(PeekMessage(&msg,NULL,0,0,PM_NOREMOVE))
		{
			if(!GetMessage(&msg,NULL,0,0)) break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//��� ���
		int bg_start=(banul*50+(count/2))%320;
		RECT src_rect;
		SetRect(&src_rect,bg_start,0,320,180);
		jdd->DrawPicture(SCREEN_BUFFER,"Sky",0,0,&src_rect);
		if(bg_start!=0)
		{
			SetRect(&src_rect,0,0,bg_start,180);
			jdd->DrawPicture(SCREEN_BUFFER,"Sky",320-bg_start,0,&src_rect);
		}

		//���������� ó��
		m_sv.var[VAR_BANUL_SPEED]=banul_speed;

		//�浹üũ
		//��
		for(int i=ENEMY_SP; i<MISSILE_SP; i++)
			if(spr[i].life)
			{
				if(Crush(0,i))
				{
					int damage=Max(0,(DATA_POW(spr[i].type)*banul_speed)-DATA_POW(spr[0].type));
					spr[0].Hurt(damage);
					spr[i].Hurt(DATA_POW(0));
					banul_speed=Max(1,banul_speed-damage/3);
				}
			}
		//�� �̻���
		for(int i=ENEMY_MISSILE_SP; i<SPR_MAX; i++)
			if(spr[i].life)
			{
				//���а˻�
				for(int sh=SHIELD_SP; sh<ENEMY_SP; sh++)
				{
					if(spr[sh].life && Crush(sh,i))spr[i].life=false;
				}
				//���ΰ� �˻�
				if(spr[i].life && Crush(0,i))
				{
					int damage=Max(0,(DATA_POW(spr[i].type)*banul_speed)-DATA_POW(spr[0].type));
					spr[0].Hurt(damage);
					if(spr[i].hp>=0)spr[i].life=false;
					banul_speed=Max(1,banul_speed-damage/3);
				}
			}
		//�Ʊ� �̻���
		for(int i=MISSILE_SP; i<ENEMY_MISSILE_SP; i++)
		{
			if(spr[i].life)
			{
				for(int j=ENEMY_SP; j<MISSILE_SP; j++)
				if(spr[j].life)
				{
					if(Crush(i,j))
					{
						spr[j].Hurt(DATA_POW(spr[i].type));
						if(spr[i].hp>=0)spr[i].life=false;
					}
				}
			}
		}

		//�� ����
		int army_command=spr[0].Control();
		if(army_command==999)banul_speed=Max(1,banul_speed-1);
		else if(army_command==2)	//3��ź
		{
			_Play(13);
			Create(CREATE_ARMY_MISSILE,0,3,51);
			Create(CREATE_ARMY_MISSILE,0,4,51);
			Create(CREATE_ARMY_MISSILE,0,5,51);
		}
		else if(army_command==5)	//16��
		{
			_Play(20);
			Create(CREATE_ARMY_MISSILE,0,8,56);
		}
		else if(army_command==6)	//��
		{
			_Play(14);
			Create(CREATE_SHIELD,0,0,57);
		}
		else if(army_command==20)	//����
		{
			_Play(18);
			spr[0].hp=Min(1000,spr[0].hp+100);
		}
		else if(army_command==21)	//����
		{
			_Play(17);
			banul_speed=max;
		}
		else if(army_command>=0)
		{
			if(army_command==0)_Play(12);
			else if(army_command==1)_Play(15);
			else if(army_command==3)_Play(19);
			else if(army_command==4)_Play(16);
			Create(CREATE_ARMY_MISSILE,0,4,army_command+51);
		}

		//ǥ��, �̵�
		for(int i=0; i<SPR_MAX; i++)
			if(spr[i].life)
			{
				int enemy_command=spr[i].Move();
				if(enemy_command>=0 && enemy_command<=15)	//���� �߻�
				{
					Create(CREATE_ENEMY_MISSILE,i,enemy_command,53);
				}
				else if(enemy_command==50)	//������ �߻�
				{
					Create(CREATE_ENEMY_MISSILE,i,rand()%16,50);
				}
				else if(enemy_command==51)	//4���� �߻�
				{
					Create(CREATE_ENEMY_MISSILE,i,2,50);
					Create(CREATE_ENEMY_MISSILE,i,6,50);
					Create(CREATE_ENEMY_MISSILE,i,10,50);
					Create(CREATE_ENEMY_MISSILE,i,14,50);
				}
				else if(enemy_command==52)	//������ �߻�
				{
					Create(CREATE_ENEMY_MISSILE,i,8,50);
				}
				else if(enemy_command==53)	//�� ������
				{
					Create(CREATE_SPR,spr[i].x+20,180,12);
				}				
				else if(enemy_command==54)	//�豸�� �߻�
				{
					Create(CREATE_ENEMY_MISSILE,i,15,52);
					Create(CREATE_ENEMY_MISSILE,i,0,52);
					Create(CREATE_ENEMY_MISSILE,i,1,52);
				}				
				spr[i].Show();
			}

		//�̺�Ʈ
		if(!m_sv.sw[SW_FIRST_DIREVE])
		{
			m_dlg.TextSnr(10);
			m_sv.sw[SW_FIRST_DIREVE]=true;
		}

		//�ð� ���
		second++;
		if(second>=100)
		{
			time++;
			spr[0].Hurt(banul_speed/4);
			second=0;
			//���ӵ�
			banul_speed=Min(banul_speed+1,max);
		}
		//�Ÿ� Ȯ��
		count+=banul_speed;
		if(count>=100)
		{
			if(stage_no<10)
			{
				banul++;
				if(banul_speed>5 && (banul%5)==0)_Play(24);
				count-=100;
				//Ŭ����
				if(banul>=stage_length)fly_end=true;

				//�������� ����
				if(stage_no<4)	//�⺻ �ٶ���
				{
					int birds[4]={10,6,5,1};
					if((banul%birds[stage_no])==0)Create(CREATE_SPR,320,rand()%140,5);
				}
				else if(stage_no==4)	//������ũ ����
				{
					Create(CREATE_SPR,320,rand()%60,7);
				}
				else if(stage_no==5)	//���� ����
				{
					if((banul%20)==0)
					{
						int create=rand()%5;
						if(create==0)Create(CREATE_SPR,320,0,9);
							else Create(CREATE_SPR,320,0,6);
					}
				}
				else if(stage_no==6)	//���� ����
				{
					if((banul%10)==0)
					{
						int create=rand()%7+5;
						if(create>=10)Create(CREATE_SPR,rand()%(320-DATA_XS(create)),180,create);	//�Ĺ�
						else if(create==5)Create(CREATE_SPR,320,rand()%140,create);					//��
						else if(create==7)Create(CREATE_SPR,320,rand()%60,create);					//������ũ
						else if(create==8)Create(CREATE_SPR,320,0,create);							//���� ��
						else Create(CREATE_SPR,320,0,create);
					}
				}
				else if(stage_no==7)	//�Ĺ�
				{
					if((banul%20)==0)
					{
						int create=rand()%5;
						if(create==0)Create(CREATE_SPR,rand()%(320-DATA_XS(create)),180,11);
							else if(create==1)Create(CREATE_SPR,320,rand()%60,7);
							else Create(CREATE_SPR,rand()%(320-DATA_XS(create)),180,10);
					}
				}
				else if(stage_no==8)	//�� ����
				{
					if(banul==1)Create(CREATE_SPR,320,0,13);
				}
				else if(stage_no==9)	//�� ��
				{
					if(banul==1)Create(CREATE_SPR,320,180,14);
				}
			}
			else
			{				
				if(!spr[ENEMY_SP].life)fly_end=true;
			}
		}
		//����â
		jdd->DrawPicture(SCREEN_BUFFER,"Control",0,180,NULL);
		jdd->DrawPicture(SCREEN_BUFFER,"Needle",107+(banul_speed*3),206,NULL);
		SetRect(&src_rect,0,0,spr[0].hp/10,10);
		jdd->DrawPicture(SCREEN_BUFFER,"Air",205,194,&src_rect);
		jdd->DrawText(SCREEN_BUFFER,StrAdd("dcdc",banul,"/",stage_length,"�ϴ�"),font12,206,206,JColor(0,0,0));
		jdd->DrawText(SCREEN_BUFFER,global_buffer,font12,205,205,JColor(255,255,255));
		jdd->DrawText(SCREEN_BUFFER,StrAdd("dcdc",time,".",second/10,"�ð�"),font12,206,221,JColor(0,0,0));
		jdd->DrawText(SCREEN_BUFFER,global_buffer,font12,205,220,JColor(255,255,255));
		//���
		for(int i=0; i<4; i++)
		{
			if(m_sv.var[VAR_EQUIP_LIST+i]<0)continue;
			jdd->DrawPicture(SCREEN_BUFFER,StrAdd("cd","���",m_sv.var[VAR_EQUIP_LIST+i]),18+21*i,205,NULL);
			jdd->DrawText(SCREEN_BUFFER,StrAdd("d",m_sv.var[VAR_EQUIP+m_sv.var[VAR_EQUIP_LIST+i]]),font10,15+21*i,224,JColor(0,0,255));
		}

		Render();
	}

	if(spr[0].life)
	{
		if(stage_no<10)
		{
			//��ϰ���
			jdd->DrawText(SCREEN_BUFFER,StrAdd("dcdc",m_sv.var[VAR_MODEL]+1,"�� �� ",stage_no,"�� �ٶ���"),font20,71,11,JColor(0,0,0));
			jdd->DrawText(SCREEN_BUFFER,global_buffer,font20,70,10,JColor(128,255,128));
			jdd->DrawText(SCREEN_BUFFER,StrAdd("cdcd","����ð�:",time,".",second),font20,71,51,JColor(0,0,0));
			jdd->DrawText(SCREEN_BUFFER,global_buffer,font20,70,50,JColor(128,255,128));
			
			int record=time*100+second;
			int rec_model=VAR_NEW_RECORD+m_sv.var[VAR_MODEL]+(stage_no*5);
			if(m_sv.var[rec_model]==0 || record<m_sv.var[rec_model])
			{
				m_sv.var[rec_model]=record;
				jdd->DrawText(SCREEN_BUFFER,StrAdd("cdcd","�ű��:",m_sv.var[rec_model]/100,".",m_sv.var[rec_model]%100),font20,71,76,JColor(0,0,0));
				jdd->DrawText(SCREEN_BUFFER,global_buffer,font20,70,75,JColor(255,255,0));
				jdd->DrawText(SCREEN_BUFFER,"��� ����!",font20,131,96,JColor(0,0,0));
				jdd->DrawText(SCREEN_BUFFER,"��� ����!",font20,130,95,JColor(255,0,0));
				
				if(stage_no<4)m_sv.var[VAR_PRIZE]+=Max(0,(stage_length-time)*20);
					else m_sv.var[VAR_PRIZE]+=Max(0,(stage_length-time)*60);
				jdd->DrawText(SCREEN_BUFFER,StrAdd("cdc","���:",m_sv.var[VAR_PRIZE],"ī��"),font20,131,116,JColor(0,0,0));
				jdd->DrawText(SCREEN_BUFFER,global_buffer,font20,130,115,JColor(128,0,0));
				if(stage_no>=4 && stage_no<=9 && !m_sv.sw[SW_GET_ITEM+(stage_no-4)])
				{
					jdd->DrawText(SCREEN_BUFFER,StrAdd("cc","��ǰ: ",item_name[stage_no-4]),font20,131,136,JColor(0,0,0));
					jdd->DrawText(SCREEN_BUFFER,global_buffer,font20,130,135,JColor(128,0,0));
					m_sv.sw[SW_GET_ITEM+(stage_no-4)]=true;
					m_sv.sw[SW_ITEM+(stage_no-4)]=true;
				}
				m_sv.var[VAR_MONEY]+=m_sv.var[VAR_PRIZE];
			}
			else
			{
				jdd->DrawText(SCREEN_BUFFER,StrAdd("cdcd","�ű��:",m_sv.var[rec_model]/100,".",m_sv.var[rec_model]%100),font20,71,76,JColor(0,0,0));
				jdd->DrawText(SCREEN_BUFFER,global_buffer,font20,70,75,JColor(255,255,255));
			}

			Render();
			Pause();
		}
		//��� �� �̺�Ʈ
		else
		{
			FadeOut();
		}
	}
	else
	{
		jdd->DrawText(SCREEN_BUFFER,"G A M E   O V E R",font32,17,21,JColor(0,0,0));
		jdd->DrawText(SCREEN_BUFFER,"G A M E   O V E R",font32,16,20,JColor(255,0,0));

		Render();
		Pause();
	}


	if(spr[0].life)
	{
		//�̺�Ʈ
		if(m_sv.var[VAR_SOLUTION]==1)
		{
			m_dlg.TextSnr(18);
			m_sv.var[VAR_SOLUTION]=2;
		}
		else if(m_sv.var[VAR_SOLUTION]==2 && m_sv.sw[SW_ITEM+2])
		{
			m_dlg.TextSnr(11);
			m_sv.var[VAR_SOLUTION]=3;
		}


		m_sv.var[VAR_SPOT]=m_sv.var[VAR_TARGET];					//����̵�
		//����
		int recover=0;
		if(m_party.IsIt(14))recover+=350;
		if(m_party.IsIt(15))recover+=400;
		if(m_party.IsIt(19))recover+=250;
		
		m_sv.var[VAR_AIR]=Min(1000,spr[0].hp+recover);//���� ���� ��
	}
	else time=-999;

	if(spr[0].life)_MidiPlay(bgm[m_sv.var[VAR_SPOT]]);
	
	jdd->DrawPicture(SCREEN_BUFFER,"Dlg",0,160,NULL);
	return time;
}