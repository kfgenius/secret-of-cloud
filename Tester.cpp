#include <dsound.h>

#include "JDirectDraw.h"
#include "Dsutil.h"
#include "JResourceManager.h"
#include "resource.h"
//#include "extern.h"

#include <math.h>
#include <time.h>
#include <stdio.h>
#include <crtdbg.h>
#include <gdiplus.h>

using namespace Gdiplus;

JDirectDraw* jdd;
JResourceManager* jre;

char* backbuffer;
JFont font20, font12, font10, font32;
MSG msg;

LRESULT CALLBACK WndProc(HWND wnd,UINT msg,WPARAM wParam,LPARAM lParam);

#include "data.h"

bool window_mode = true;

//�ٶ��� ����
int SelectLine(int start)
{
	//�� �������� �̾����� �ٶ���
	int to[5];
	switch(start)
	{
		case 0: to[0]=-1; to[1]=0; to[2]=1; to[3]=2; to[4]=3; break;
		case 1: to[0]=0; to[1]=-1; to[2]=4; to[3]=5; to[4]=6; break;
		case 2: to[0]=1; to[1]=4; to[2]=-1; to[3]=7; to[4]=8; break;
		case 3: to[0]=2; to[1]=5; to[2]=7; to[3]=-1; to[4]=9; break;
		case 4: to[0]=3; to[1]=6; to[2]=8; to[3]=9; to[4]=-1; break;
	}

	int airport_x[5]={171,79,78,220,193};
	int airport_y[5]={74,75,39,30,130};

	//�ٶ��� ���� ����
	int selected_line=0;
	int airline_start=0;

	while(1)
	{
		if(PeekMessage(&msg,NULL,0,0,PM_NOREMOVE))
		{
			if(!GetMessage(&msg,NULL,0,0)) break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//���
		//===========================================
		jdd->DrawPicture(SCREEN_BUFFER,"WorldMap",0,0,NULL);
		//���� ǥ��
		for(int i=0; i<5; i++)
		{
			if(i==start || i==selected_line)
				jdd->DrawPicture(SCREEN_BUFFER,"Airport_s",airport_x[i]-16,airport_y[i]-16,NULL);
			else
				jdd->DrawPicture(SCREEN_BUFFER,"Airport_u",airport_x[i]-16,airport_y[i]-16,NULL);
		}
		//�ٶ��� ǥ��
		airline_start++;
		if(airline_start>19)airline_start=0;
		for(int i=airline_start; i<100; i+=20)
		{
			int bx=airport_x[start]+((airport_x[selected_line]-airport_x[start])*i/100);
			int by=airport_y[start]+((airport_y[selected_line]-airport_y[start])*i/100);
			jdd->DrawPicture(SCREEN_BUFFER,"Airline",bx-4,by-3,NULL);
		}
		Render();

		//ó������ ���� �̺�Ʈ
		if(!m_sv.sw[SW_BARAMGIL])
		{
			m_dlg.TextSnr(57);
			m_sv.sw[SW_BARAMGIL]=true;			
		}
		
		//����
		//===========================================
		//�ٶ��� ����
		if(GetKeyLeft())selected_line--;
		else if(GetKeyRight())selected_line++;
		else if(GetKeyUp())selected_line--;
		else if(GetKeyDown())selected_line++;
		
		bool test;
		do{
			test=true;
			if(selected_line > 4)selected_line=0;
			if(selected_line < 0)selected_line=4;
			if(to[selected_line] == -1)
			{
				test=false;
				selected_line++;
			}
		}while(!test);

		//����
		if(_GetKeyState(VK_RETURN))
		{
			if(!enter)
			{
				enter=true;
				break;
			}
		}
		else enter=false;

		if(_GetKeyState(VK_ESCAPE))
		{
			if(!esc)
			{
				esc=true;
				to[selected_line]=-1;
				break;
			}
		}
		else esc=false;
	}

	m_sv.var[VAR_TARGET]=selected_line;
	return to[selected_line];
}

typedef struct
{
	int id;
	int hp;
	int str;
	int agl;
	int ready;
	int state;
}Fighter;

//����è�ǿ�
int Fight(int player1, int player2)
{
	FadeOut();
	_MidiPlay(bgm[8]);
	//������ ������
	int p[2];
	p[0]=player1;
	p[1]=player2;
	//�÷��̾� ������ �Է�
	Fighter player[2];
	for(int i=0; i<2; i++)
	{
		player[i].id=p[i];
		player[i].hp=m_sv.var[VAR_HP+p[i]];
		player[i].str=m_sv.var[VAR_POW+p[i]];
		player[i].agl=m_sv.var[VAR_SPD+p[i]];
		player[i].ready=100;
		player[i].state=0;
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
		//����
		for(int i=0; i<2; i++)
		{
			//ü��
			if(player[i].hp==m_sv.var[VAR_HP+player[i].id])
				jdd->DrawText(SCREEN_BUFFER,StrAdd("cd","ü�� ",player[i].hp),font20,i*160,0,JColor(0,0,255));
			else if(player[i].hp<=(m_sv.var[VAR_HP+player[i].id]/5))
				jdd->DrawText(SCREEN_BUFFER,StrAdd("cd","ü�� ",player[i].hp),font20,i*160,0,JColor(255,0,0));
			else
				jdd->DrawText(SCREEN_BUFFER,StrAdd("cd","ü�� ",player[i].hp),font20,i*160,0,JColor(255,255,255));
			//��
			jdd->DrawText(SCREEN_BUFFER,StrAdd("cd","��   ",player[i].str),font20,i*160,20,JColor(255,255,255));
			//��ø
			jdd->DrawText(SCREEN_BUFFER,StrAdd("cd","��ø ",player[i].agl),font20,i*160,40,JColor(255,255,255));
			//�غ�
			int ready=Max(0,player[i].ready);
			if(player[i].ready<100)
				jdd->DrawText(SCREEN_BUFFER,StrAdd("cdc","�غ� ",ready,"%"),font20,i*160,60,JColor(255,255,255));
			else
				jdd->DrawText(SCREEN_BUFFER,"�غ� 100%",font20,i*160,60,JColor(255,255,0));
			//��
			jdd->DrawPicture(SCREEN_BUFFER,StrAdd("cd","Face",player[i].id+1),i*160,90,NULL);
		}
		Render();
		if(player[0].hp<=0 || player[1].hp<=0)break;	//����� ������ ���⿡ �ξ���

		for(int i=0; i<2; i++)
		{
			int army=i;
			int enemy=1-i;

			if(player[army].ready>=100)
			{
				//����&AI
				int attack;
				if(army==0)select=attack=m_dlg.TextSel(51,-1,select);
				else
				{
					if(player[enemy].ready<=50)attack=rand()%4;
					else
					{
						if((rand()%3)==0)
							attack=rand()%4;
						else
						{
							if(player[enemy].agl>player[army].agl)attack=4;
								else attack=5;
						}
					}					
				}
				//����ൿ
				if(attack==5)
				{
					player[army].state=1;	//���ϱ�
					player[army].ready-=100;
				}
				else if(attack==4)
				{
					player[army].state=2;	//����
					player[army].ready-=100;
				}
				else player[army].state=0;
				//�����ൿ
				int damage;
				if(attack>=0 && attack<=3)
				{
					//�޼���
					if(attack==0)m_dlg.TextPut(StrAdd("ccc","-1000",fighter[player[army].id],"�� ������ �����̴�."));
					else if(attack==1)m_dlg.TextPut(StrAdd("ccc","-1000",fighter[player[army].id],"�� ���� �����̴�."));
					else if(attack==2)m_dlg.TextPut(StrAdd("ccc","-1000",fighter[player[army].id],"�� ���� �����̴�."));
					else m_dlg.TextPut(StrAdd("ccc","-1000",fighter[player[army].id],"�� �ʻ� �����̴�."));

					int attack_rate[]={1,2,4,6};
					damage=(3+player[army].str)*attack_rate[attack]+rand()%6;
					player[army].ready-=50*(attack+1);
					//����� ����
					//�����
					if(player[enemy].state==0)
					{
						_Play(attack+2);
						player[enemy].hp=Max(0,player[enemy].hp-damage);
						m_dlg.TextPut(StrAdd("cccdc","-1000",fighter[player[enemy].id],"�� ",damage,"�������� �Ծ���."));
					}
					//���ϱ�
					else if(player[enemy].state==1)
					{
						_Play(8);
						m_dlg.TextPut(StrAdd("ccc","-1000",fighter[player[enemy].id],"�� ���ߴ�."));
						player[enemy].state=0;
					}
					//����
					else if(player[enemy].state==2)
					{
						_Play(9);
						damage/=2;
						player[enemy].hp=Max(0,player[enemy].hp-damage);
						m_dlg.TextPut(StrAdd("cccdc","-1000",fighter[player[enemy].id],"�� ���Ҵ�.\\",damage,"�������� �Ծ���."));
					}
				}
				else if(attack==6)
				{
					player[army].hp=Min(m_sv.var[VAR_HP+player[army].id],player[army].hp+10);
					player[army].ready-=100;
				}
			}
			//�غ��ϱ�
			player[army].ready+=(player[army].agl+3);
		}
	}

	//����
	int winlose;
	if(player[0].hp==0 && player[1].hp==0)	//���º�
	{
		_Play(7);
		m_dlg.TextSnr(84);
		winlose=0;
	}
	else if(player[0].hp==0)	//�й�
	{
		_Play(0);
		m_dlg.TextSnr(85);
		winlose=-1;
	}
	else	//�¸�
	{
		_Play(6);
		m_dlg.TextSnr(86);
		winlose=1;
	}
	
	//�Ƿ� ���
	int id=player1;
	m_sv.var[VAR_HP+id]+=15;
	if((rand()%2)==0)m_sv.var[VAR_POW+id]++;
		else m_sv.var[VAR_SPD+id]++;
	id=player2;
	m_sv.var[VAR_HP+id]+=15;
	if((rand()%2)==0)m_sv.var[VAR_POW+id]++;
		else m_sv.var[VAR_SPD+id]++;

	FadeOut();
	_MidiPlay(bgm[m_sv.var[VAR_SPOT]]);

	return winlose;
}

//���� �ָ���
int Gamble()
{
	bool mycard[16];
	bool dealer[16];
	int score[2]={0,0};

	for(int i=0; i<16; i++)
	{
		mycard[i]=true;
		dealer[i]=true;
	}
	
	int use_card, dealer_card;
	for(int shot=0; shot<16; shot++)
	{
		for(int j=0; j<8; j++)
			jdd->DrawPicture(SCREEN_BUFFER,"Black",0,j*20,NULL);

		//������
		int put_money=m_sv.var[VAR_MONEY];
		int px=276;
		while(put_money>=10)
		{
			put_money/=10;
			px-=11;
		}
		jdd->DrawText(SCREEN_BUFFER,StrAdd("dc",m_sv.var[VAR_MONEY],"ī��"),font12,px+1,140,JColor(0,0,0));
		jdd->DrawText(SCREEN_BUFFER,StrAdd("dc",m_sv.var[VAR_MONEY],"ī��"),font12,px,139,JColor(255,255,0));

		//��
		jdd->DrawPicture(SCREEN_BUFFER,"Face0",0,90,NULL);
		jdd->DrawText(SCREEN_BUFFER,StrAdd("d",score[0]),font20,55,140,JColor(255,255,255));
		jdd->DrawPicture(SCREEN_BUFFER,"Face30",265,0,NULL);
		if(score[1]<10)
			jdd->DrawText(SCREEN_BUFFER,StrAdd("d",score[1]),font20,253,50,JColor(255,255,255));
		else
			jdd->DrawText(SCREEN_BUFFER,StrAdd("d",score[1]),font20,241,50,JColor(255,255,255));

		//�ڱ� ī�� �̱�
		CCommand m_card(1);
		bool first=true;
		for(int i=0; i<16; i++)
			if(mycard[i])m_card.AddCom(i);

		use_card=m_card.GetCommand();
		mycard[use_card]=false;

		//������ ī��
		do{
			dealer_card=rand()%16;
		}while(!dealer[dealer_card]);
		dealer[dealer_card]=false;

		if(use_card<12 && dealer_card<12)	//���� �º�
		{
			if(use_card > dealer_card)score[0]+=2;
				else if(use_card < dealer_card)score[1]+=2;
		}
		else if(use_card>=12 && dealer_card<12)	//Ư��ī�� �º�
		{
			if(use_card==12 && (dealer_card%2)==0)score[0]+=2;
				else if(use_card==13 && (dealer_card%2)==1)score[0]+=2;
				else if(use_card==14 && dealer_card<=5)score[0]+=2;
				else if(use_card==15 && dealer_card>=6)score[0]+=2;
				else score[1]+=2;
		}
		else if(use_card<12 && dealer_card>=12)	//������ Ư��ī�� �º�
		{
			if(dealer_card==12 && (use_card%2)==0)score[1]+=2;
				else if(dealer_card==13 && (use_card%2)==1)score[1]+=2;
				else if(dealer_card==14 && use_card<=5)score[1]+=2;
				else if(dealer_card==15 && use_card>=6)score[1]+=2;
				else score[0]+=2;
		}

		jdd->DrawPicture(SCREEN_BUFFER,StrAdd("cd","Card",use_card),140,90,NULL);
		jdd->DrawPicture(SCREEN_BUFFER,StrAdd("cd","Card",dealer_card),140,10,NULL);
		Render();
		Sleep(1000);
	}
	//������ ��� �����ֱ�
	for(int j=0; j<8; j++)
		jdd->DrawPicture(SCREEN_BUFFER,"Black",0,j*20,NULL);
	int put_money=m_sv.var[VAR_MONEY];
	int px=276;
	while(put_money>=10)
	{
		put_money/=10;
		px-=11;
	}
	jdd->DrawText(SCREEN_BUFFER,StrAdd("dc",m_sv.var[VAR_MONEY],"ī��"),font12,px+1,140,JColor(0,0,0));
	jdd->DrawText(SCREEN_BUFFER,StrAdd("dc",m_sv.var[VAR_MONEY],"ī��"),font12,px,139,JColor(255,255,0));

	jdd->DrawPicture(SCREEN_BUFFER,"Face0",0,90,NULL);
	jdd->DrawText(SCREEN_BUFFER,StrAdd("d",score[0]),font20,55,140,JColor(255,255,255));
	jdd->DrawPicture(SCREEN_BUFFER,"Face30",265,0,NULL);
	if(score[1]<10)
		jdd->DrawText(SCREEN_BUFFER,StrAdd("d",score[1]),font20,253,50,JColor(255,255,255));
	else
		jdd->DrawText(SCREEN_BUFFER,StrAdd("d",score[1]),font20,241,50,JColor(255,255,255));
	jdd->DrawPicture(SCREEN_BUFFER,StrAdd("cd","Card",use_card),140,90,NULL);
	jdd->DrawPicture(SCREEN_BUFFER,StrAdd("cd","Card",dealer_card),140,10,NULL);

	int victory=score[0]-score[1];
	if(victory>0)m_dlg.TextPut(StrAdd("cdc","-1000����� ",victory,"�� ���� �̰���ϴ�."));
		else if(victory<0)m_dlg.TextPut(StrAdd("cdc","-1000����� ",abs(victory),"�� ���� �����ϴ�."));
		else m_dlg.TextPut("-1000�����ϴ�.");

	return victory;
}

//����
void AirPort()
{
	if(m_dlg.TextSel(8)==0)
	{
		int baramgil;
		if(!m_sv.sw[SW_START])
		{
			m_dlg.TextSnr(9);
			m_sv.sw[SW_START]=true;
			m_sv.var[VAR_SOLUTION]=1;
			baramgil=0;
			m_sv.var[VAR_TARGET]=1;
		}
		else
		{
			//���
			if(!m_sv.sw[SW_SHIP_BATTLE] && m_sv.var[VAR_SOLUTION]==3)
			{
				m_dlg.TextSnr(143);
				if(m_dlg.TextSel(144)==0)
				{
					m_dlg.TextSnr(146);
					baramgil=10;
					m_sv.sw[SW_SHIP_BATTLE]=true;
					m_sv.var[VAR_TARGET]=m_sv.var[VAR_SPOT];
				}
				else
				{
					m_dlg.TextSnr(145);
					baramgil=SelectLine(m_sv.var[VAR_SPOT]);
				}
			}
			else baramgil=SelectLine(m_sv.var[VAR_SPOT]);
		}
		//0�̸��� ���� ���
		if(baramgil>=0)
		{
			FadeOut();
			CShooting m_shoot(baramgil);
			
			int fly_result=m_shoot.Play();
			if(fly_result==-999)m_sv.sw[SW_QUIT]=true;
				else
				{
					m_game.Time(fly_result);
					m_sv.sw[SW_GANG]=true;

					//ǳ�� ��� �¸�
					if(!m_sv.sw[SW_GET_WATER] && m_sv.sw[SW_SHIP_BATTLE])
					{
						m_dlg.TextSnr(147);
						m_sv.sw[SW_ITEM+7]=true;
						m_sv.sw[SW_GET_WATER]=true;
					}
				}
		}
	}
}

//���� �湮���� �� �Ͼ�� ������ �̺�Ʈ
void NormalHouse(int who)	//job: 0 - �Ϲ� ����, 1 - ������, 3 - ����, 4-�ư��̹�
{
	int fee;		//�ʿ��� ��
	int ab1, ab2;	//�ɷ�
	int list[5];	//���� �ִ� �͵�

	int base_talk, sign_talk, not_talk;		//��ȭ ����
	int job=0;		//����
	int tribe=0;	//����
	int jewel;		//���� ���� ����

	switch(who)
	{
		//Ŭ���
		case 2: fee=52000;
			base_talk=46; sign_talk=63;
			break;
		//������
		case 3: fee=30000;
			tribe=1;
			base_talk=45; not_talk=44; sign_talk=64;
			break;
		//����
		case 4: fee=13000;
			tribe=1;
			base_talk=60; not_talk=23; sign_talk=65;
			break;
		//�̾�
		case 5: fee=68000;
			base_talk=24; sign_talk=66;
			break;
		//���
		case 6: fee=33000;
			tribe=2;
			base_talk=59; not_talk=25; sign_talk=67;
			break;
		//����
		case 9: fee=200;
			base_talk=37; sign_talk=68;
			break;
		//����
		case 10: fee=100;
			base_talk=38; sign_talk=69;
			break;
		//������
		case 11: fee=8000;
			base_talk=26; sign_talk=70;
			break;
		//���
		case 13: fee=5000;
			base_talk=30; sign_talk=71;
			break;
		//����
		case 14: fee=50000;
			tribe=2; job=1;
			base_talk=61; not_talk=39; sign_talk=72;
			break;
		//����
		case 15: fee=60000;
			tribe=2; job=1;
			base_talk=48; not_talk=47; sign_talk=73;
			break;
		//����
		case 19: fee=80000;
			tribe=1; job=4;
			base_talk=31; not_talk=36; sign_talk=74;
			break;
		//���̾�
		case 23: fee=3000;
			tribe=2;
			base_talk=50; not_talk=49; sign_talk=75;
			break;
		//����
		case 24: fee=4500;
			tribe=1;
			base_talk=41; not_talk=40; sign_talk=76;
			break;
		//�����
		case 28: fee=40000;	ab1=0; ab2=1;
			base_talk=16; sign_talk=21;
			list[0]=0; list[1]=1; list[2]=3; list[3]=20; list[4]=21;
			job=3; jewel=30000;
			break;
		//����
		case 29: fee=30000;	ab1=2; ab2=-1;
			list[0]=2; list[1]=4; list[2]=5; list[3]=20; list[4]=21;
			tribe=1; job=3; jewel=40000;
			base_talk=62; not_talk=27; sign_talk=77;
			break;
		//����
		case 31: fee=100000; ab1=3; ab2=-1;
			list[0]=3; list[1]=4; list[2]=5; list[3]=6; list[4]=21;
			tribe=2; job=3; jewel=20000;
			base_talk=43; not_talk=42; sign_talk=78;
			break;
	}

	if(!m_party.IsIt(who))
	{
		int talkable=false;
		switch(tribe)
		{
			case 0: talkable=true; break;
			case 1: if(TRIBE_SKY)talkable=true; break;
			case 2: if(TRIBE_LIGHTNING)talkable=true; break;
		}

		if(talkable)
		{
			//�������� ����
			if(job==3 && m_sv.sw[SW_ITEM+5])
			{
				m_dlg.TextPut(StrAdd("dcdc",who,"999����! �װ� ���� ����.\\",jewel,"ī���� ������ �ļ���."));
				int decision=m_dlg.TextSel(108);
				if(decision==0)
				{
					m_dlg.TextPut(StrAdd("dc",who,"999�����ϴ�."));
					m_sv.var[VAR_MONEY]+=jewel;
					m_sv.sw[SW_ITEM+5]=false;
				}
			}
			m_dlg.TextSnr(base_talk);	//�⺻��ȭ
			CCommand m_house(0);
			//�������� Ư�����
			if(job==3)m_house.AddCom(2);	//����
				else if((job==1 || job==4) && m_sv.var[VAR_AIR]<1000)m_house.AddCom(5);	//����
			//������ ���
			if(m_sv.var[VAR_PARTY_NUM]<8+m_sv.var[VAR_PARTY_ADD])m_house.AddCom(1);
			//������ ���
			m_house.AddCom(0);
			//��ɾ� �ޱ�
			int com=m_house.GetCommand();

			//������
			if(com==1)
			{
				m_dlg.TextPut(StrAdd("cdc","-1999����:",fee,"ī��"));
				int yn=m_dlg.TextSel("����� ��´�.\\���д�.");
				if(yn==0)
				{
					if(m_sv.var[VAR_MONEY]>=fee)
					{
						m_dlg.TextSnr(sign_talk);	//�λ�
						m_party.Insert(who);
						m_sv.var[VAR_MONEY]-=fee;

						//������ ��� ���⸦ ���Ѵ��
						if(job==3)
						{
							m_sv.var[VAR_EQUIP+ab1]=99;
							if(ab2>0)m_sv.var[VAR_EQUIP+ab2]=99;
						}
					}
					else m_dlg.TextSnr(12);
				}
			}
			//����
			else if(com==2)
			{
				m_game.Shop(FIVE,list[0],list[1],list[2],list[3],list[4]);
			}
			//����
			else if(com==5)
			{
				int fix=1000-m_sv.var[VAR_AIR];
				m_dlg.TextPut(StrAdd("cdc","-1999������:",fix,"ī��"));
				int yn=m_dlg.TextSel("�����Ѵ�.\\���д�.");
				if(yn==0)
				{
					if(m_sv.var[VAR_MONEY]>=fix)
					{
						m_sv.var[VAR_MONEY]-=fix;
						m_sv.var[VAR_AIR]=1000;
					}
					else m_dlg.TextSnr(12);
				}
			}
		}
		else m_dlg.TextSnr(not_talk);
	}
	else m_dlg.TextSnr(164);

}

int GetRank(int who)
{
	for(int i=0; i<10; i++)
		if(m_sv.var[VAR_RANK+i]==who)return i;

	return -1;
}

void MakeGlass()
{
	bool make=false;
	if(m_sv.sw[SW_SEED_MONEY])
	{
		m_dlg.TextSnr(157);
		CCommand m_glass(9);			
		if(m_sv.var[VAR_MONEY]>=50000)m_glass.AddCom(0);
		m_glass.AddCom(1);
		int decision=m_glass.GetCommand();
		if(decision==0)
		{
			m_sv.var[VAR_MONEY]-=50000;
			make=true;
		}
	}
	else make=true;

	if(make)
	{
		m_dlg.TextSnr(156);
		m_sv.sw[SW_ITEM+1]=false;
		m_sv.sw[SW_ITEM+4]=false;
		m_sv.sw[SW_ITEM+8]=true;
	}
}

//int main()
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstancem, LPSTR lpCmdLine, int nShowCmd)
{
	jdd=CreateDirectDraw();
	jre=CreateDXResourceManager(jdd);

	//HINSTANCE hInstance=(HINSTANCE)0x00400000;

	WNDCLASS wc={0};
	wc.hIcon=LoadIcon(hInstance,"CLOUD.ico");
	wc.hCursor=LoadCursor(hInstance,IDC_ARROW);
	wc.lpfnWndProc=WndProc;
	wc.hInstance=hInstance;
	wc.style=CS_HREDRAW|CS_VREDRAW;
	wc.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszClassName="Game";
	RegisterClass(&wc);

	if(window_mode)
	{
		LONG ws=WS_OVERLAPPEDWINDOW|WS_VISIBLE;
		ws &= ~WS_THICKFRAME;
		ws &= ~WS_MAXIMIZEBOX;

		RECT crt;
		SetRect(&crt, 0, 0, SCREEN_X, SCREEN_Y);
		AdjustWindowRect(&crt, ws, FALSE);

		hwnd = CreateWindow("Game", "ū������ ��������", ws, 100, 100, crt.right - crt.left, crt.bottom - crt.top, NULL, NULL, hInstance, NULL);
		ShowCursor( TRUE );
	}
	else
	{
		hwnd=CreateWindow("Game","ū������ ��������",WS_POPUP|WS_VISIBLE,0,0,640,480,NULL,NULL,hInstance,NULL);
		ShowCursor( FALSE );
	}

	if ( DirectSoundCreate(NULL,&SoundOBJ,NULL) == DS_OK )
	{
		SoundCard = TRUE;
		if (SoundOBJ->SetCooperativeLevel(hwnd,DSSCL_PRIORITY)!=DS_OK) return 0;

		memset(&DSB_desc,0,sizeof(DSBUFFERDESC));
		DSB_desc.dwSize = sizeof(DSBUFFERDESC);
		DSB_desc.dwFlags = DSBCAPS_PRIMARYBUFFER;

		if (SoundOBJ->CreateSoundBuffer(&DSB_desc,&SoundDSB,NULL)!=DS_OK) return 0;
		SoundDSB -> SetVolume(0);
		SoundDSB -> SetPan(0);
	}
	else SoundCard = FALSE;

	jdd->Initialize(NULL,hwnd,SCREEN_X,SCREEN_Y,16,true,window_mode);
	
	//�ӽ� �����̽� ����
	JPictureInfo jpi;
	jpi.SetWidth(SCREEN_X);
	jpi.SetHeight(SCREEN_Y);
	jdd->CreateSurface(SCREEN_BUFFER, &jpi, TRUE);

	//������â �̵�
	if(window_mode)
	{
		jdd->OnMove(100, 100);
		SetCursor(LoadCursor(0, IDC_ARROW));
	}

	//�ʱ�ȭ
	backbuffer = jdd->GetBackBuffer();
	font20=jdd->CreateFont("����ü",20,true,false,false,false,false);
	font12=jdd->CreateFont("HY����L",15,true,false,false,false,false);
	font10=jdd->CreateFont("HY�߰��",12,false,false,false,false,false);
	font32=jdd->CreateFont("����ü",32,true,false,false,false,false);
	srand( (unsigned)time( NULL ) );
	
	jdd->SetFrameRate(100,true);
	jdd->SetVerticalSync(false);

	//�׸� �ҷ�����
	jre->LoadResource("data\\face.mlc");
	jre->LoadResource("data\\equip.mlc");
	jre->LoadResource("data\\shooting.mlc");
	jre->LoadResource("data\\card.mlc");
	jre->LoadResource("data\\etc.mlc");

	//���� �ʱ�ȭ
	if(SoundCard)
	{
		Sound[0] = SndObjCreate(SoundOBJ,"sound\\scream1.wav",2);
		Sound[1] = SndObjCreate(SoundOBJ,"sound\\scream2.wav",2);
		Sound[2] = SndObjCreate(SoundOBJ,"sound\\p1.wav",2);
		Sound[3] = SndObjCreate(SoundOBJ,"sound\\p2.wav",2);
		Sound[4] = SndObjCreate(SoundOBJ,"sound\\p3.wav",2);
		Sound[5] = SndObjCreate(SoundOBJ,"sound\\p4.wav",2);
		Sound[6] = SndObjCreate(SoundOBJ,"sound\\win.wav",2);
		Sound[7] = SndObjCreate(SoundOBJ,"sound\\tie.wav",2);
		Sound[8] = SndObjCreate(SoundOBJ,"sound\\escape.wav",2);
		Sound[9] = SndObjCreate(SoundOBJ,"sound\\guard.wav",2);
		Sound[10] = SndObjCreate(SoundOBJ,"sound\\sell.wav",2);
		Sound[11] = SndObjCreate(SoundOBJ,"sound\\rain.wav",2);
		Sound[12] = SndObjCreate(SoundOBJ,"sound\\shot.wav",2);
		Sound[13] = SndObjCreate(SoundOBJ,"sound\\3shot.wav",2);
		Sound[14] = SndObjCreate(SoundOBJ,"sound\\shield.wav",2);
		Sound[15] = SndObjCreate(SoundOBJ,"sound\\kingball.wav",2);
		Sound[16] = SndObjCreate(SoundOBJ,"sound\\nose.wav",2);
		Sound[17] = SndObjCreate(SoundOBJ,"sound\\rocket.wav",2);
		Sound[18] = SndObjCreate(SoundOBJ,"sound\\air.wav",2);
		Sound[19] = SndObjCreate(SoundOBJ,"sound\\laser.wav",2);
		Sound[20] = SndObjCreate(SoundOBJ,"sound\\fall.wav",2);
		Sound[21] = SndObjCreate(SoundOBJ,"sound\\explosion.wav",2);
		Sound[22] = SndObjCreate(SoundOBJ,"sound\\die.wav",2);
		Sound[23] = SndObjCreate(SoundOBJ,"sound\\monster.wav",2);
		Sound[24] = SndObjCreate(SoundOBJ,"sound\\wind.wav",2);
	}

	/*���ÿ� ĳ���� ���� �ʱ�ȭ
						(�̸�	   �� �ӵ�  �����ð�)*/
	spr_data[0].SetData("Ship1",	1,	2,	1000);
	spr_data[1].SetData("Ship2",	1,	4,	1000);
	spr_data[2].SetData("Ship3",	2,	5,	1000);
	spr_data[3].SetData("Ship4",	1,	7,	1000);
	spr_data[4].SetData("Ship5",	4,	3,	1000);
	spr_data[5].SetData("Bird",		1,	1,	1);
	spr_data[6].SetData("Horneye",	1,	1,	10);
	spr_data[7].SetData("Spike",	1,	1,	25);
	spr_data[8].SetData("BirdRed",	1,	1,	5);
	spr_data[9].SetData("HorneyeRed",1,	2,	10);
	spr_data[10].SetData("Plant",	1,	1,	5);
	spr_data[11].SetData("PlantRed",1,	1,	10);
	spr_data[12].SetData("Flower",	1,	1,	1);
	spr_data[13].SetData("HorneyeKing",1,1,	10000);
	spr_data[14].SetData("FlowerKing",1,1,	10000);
	spr_data[15].SetData("Ship3",	2,	5,	1000);
	
	spr_data[50].SetData("Shot",	3,	1,	200);
	spr_data[51].SetData("����0",	1,	2,	300);
	spr_data[52].SetData("����1",	1,	1,	-1);
	spr_data[53].SetData("Shot2",	5,	2,	200);
	spr_data[54].SetData("����3",	10,	2,	200);
	spr_data[55].SetData("����4",	2,	1,	-1);
	spr_data[56].SetData("����5",	1,	3,	-1);
	spr_data[57].SetData("����6",	1,	1,	500);

	m_sv.sw[SW_QUIT]=true;
	bool end=false;
	while(!end)
	{
		//���� ����
		int cx=5, cy=2;
		while(!m_sv.sw[SW_QUIT])
		{
			if(PeekMessage(&msg,NULL,0,0,PM_NOREMOVE))
			{
				if(!GetMessage(&msg,NULL,0,0)) break;
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			if(GetKeyLeft())cx=Max(0,cx-1);
			if(GetKeyRight())cx=Min(9,cx+1);
			if(GetKeyUp())cy=Max(0,cy-1);
			if(GetKeyDown())cy=Min(4,cy+1);

			//ġƮ
			/*if(_GetKeyState('H'))m_game.Time(24);
			if(_GetKeyState('M'))m_sv.var[VAR_MONEY]+=10000;
			if(_GetKeyState('N'))m_sv.var[VAR_MONEY]-=10000;
			if(_GetKeyPush('G'))
			{
				m_sv.var[VAR_SPOT]++;
				if(m_sv.var[VAR_SPOT]>=6)m_sv.var[VAR_SPOT]=0;
			}*/

			//�̺�Ʈ
			if(_GetKeyState(VK_RETURN))
			{
				if(!enter)
				{
					enter=true;
					//�̺�Ʈ��
					////////////////////////////////////////////////
					//ȥ�� ����
					if(m_sv.var[VAR_SPOT]==0)
					{
						if(cx==5 && cy==2)//���ΰ� ��
						{
							m_dlg.TextSnr(4);
							int pass=m_dlg.TextSel(88);
							if(pass<=2)
							{
								if(pass==0)m_game.Time(1);
									else if(pass==1)m_game.Day(1);
									else if(pass==2)m_game.Day(7);
								FadeOut();
							}
						}
						else if(cx==5 && cy==3) //������ ��
						{
							if(m_sv.sw[SW_ITEM+8])
							{
								FadeOut();
								if(!m_sv.sw[SW_FIRST_DOOR])
								{
									m_dlg.TextSnr(160);
									m_sv.sw[SW_FIRST_DOOR]=true;
								}
								else if(m_sv.var[VAR_SOLUTION]==8)
								{
									m_dlg.TextSnr(174);
									m_sv.var[VAR_SOLUTION]=9;
								}

								m_sv.var[VAR_SPOT]=5;
								_MidiPlay(bgm[m_sv.var[VAR_SPOT]]);
							}
						}
						else if(cx==2 && cy==2)//������
						{
							NormalHouse(28);
						}
						else if(cx==3 && cy==3)//������
						{
							CCommand m_our_fighter(3);
							for(int i=1; i<=10; i++)
								if(m_party.IsIt(i))m_our_fighter.AddCom(i-1);
							
							while(1)
							{
								CCommand m_stadium(2);
								if(!m_our_fighter.Empty())m_stadium.AddCom(1);
								m_stadium.AddCom(0);
								int com=m_stadium.GetCommand();
								if(com==1)
								{
									m_dlg.TextSnr(82);
									int army=m_our_fighter.GetCommand()+1;
									//�������� ��ũ �˱�
									int our_rank;
									our_rank=GetRank(army);
									//������ ���� ã��
									CCommand m_challenge(4);
									for(int i=1; i<=3; i++)
									{
										int challenge_rank=our_rank-i;
										if(challenge_rank<0 ||
											m_party.IsIt(m_sv.var[VAR_RANK+challenge_rank]))continue;
										else if(challenge_rank==0)
											m_challenge.AddCom(4);
										else
											m_challenge.AddCom(i);
									}

									if(m_challenge.Empty())m_dlg.TextSnr(83);
									else
									{
										m_challenge.AddCom(0);	//���
										com=m_challenge.GetCommand();
										if(com>0)
										{
											int fee;
											if(com<4)fee=com*1000;
												else fee=10000;
											if(m_sv.var[VAR_MONEY]<fee)
												m_dlg.TextSnr(12);	//�� ����
											else
											{
												m_sv.var[VAR_MONEY]-=fee;
												m_game.Day(10);

												int enemy;
												if(com==4)enemy=m_sv.var[VAR_RANK]; 
													else enemy=m_sv.var[VAR_RANK+our_rank-com];
												int vs=Fight(army-1,enemy-1);
												//�¸����� �� ���� ����
												if(vs==1)
												{
													int tmp, prize;
													tmp=m_sv.var[VAR_RANK+our_rank];
													m_sv.var[VAR_RANK+our_rank]=enemy;
													if(com==4)	//è�ǿ� �Ǵ�
													{
														m_dlg.TextSnr(87);
														m_sv.var[VAR_RANK]=tmp;
														prize=100000;
													}
													else	//��ũ��
													{
														m_sv.var[VAR_RANK+our_rank-com]=tmp;
														prize=50000-((our_rank-com)*5000);
													}
													m_dlg.TextPut(StrAdd("cdc","-1999���:",prize,"ī��"));
													m_sv.var[VAR_MONEY]+=prize;
												}
											}
										}
									}
								}
								else break;
							}
						}
						else if(cx==4 && cy==2)//�ƽ���
						{
							if(m_sv.sw[SW_VISIT_MAX])	//�ƽ��� ���ᰡ �� ��
							{
								m_dlg.TextSnr(5);
							}
							else
							{
								m_dlg.TextSnr(1);
								m_sv.sw[SW_VISIT_MAX]=true;
								m_party.Insert(20);
							}
						}
						else if(cx==7 && cy==3)	//ǳ������
						{
							if(m_sv.sw[SW_GET_BALLOON])
							{
								if(!m_sv.sw[SW_WATCH] && m_sv.sw[SW_SHIP_BATTLE])
								{
									m_dlg.TextSnr(152);
									m_sv.sw[SW_WATCH]=true;
								}
								else if(m_sv.sw[SW_ITEM+5])
								{
									if(m_sv.sw[SW_CLOUD_JEWEL])m_dlg.TextSnr(113);
										else m_dlg.TextSnr(109);
									int decision=m_dlg.TextSel(108);
									if(decision==0)
									{
										m_dlg.TextSnr(112);
										if(m_sv.sw[SW_CLOUD_JEWEL])m_sv.var[VAR_MONEY]+=50000;
											else m_sv.var[VAR_MONEY]+=10000;
										m_sv.sw[SW_ITEM+5]=false;
									}
									else m_sv.sw[SW_CLOUD_JEWEL]=true;
								}

								m_dlg.TextSnr(7);
								CCommand m_bshop(0);
								if(!m_sv.sw[SW_LENT_END] && m_sv.var[VAR_LENT_FEE]>0)m_bshop.AddCom(7);
								if(m_sv.var[VAR_SOLUTION]>=9)m_bshop.AddCom(8);
								m_bshop.AddComs(TWO,3,0);

								int com=m_bshop.GetCommand();
								//ǳ�����
								if(com==3)
								{
									if(!m_sv.sw[SW_BUY_BALLOON])
									{
										m_dlg.TextSnr(81);
										m_sv.sw[SW_BUY_BALLOON]=true;
									}									
									m_game.BalloonShop();
								}
								//�Ӵ�� ����
								else if(com==7)
								{
									m_dlg.TextSnr(79);
								}
								//���������� �ش�
								else if(com==8)
								{
									int com=0;
									int i;
									for(i=0; i<8; i++)
									{
										answer_max=i;
										com=m_dlg.TextSel(80,BACK_END,com);
										if(com==26)i=Max(i-2,-1);
											else if(com==27)break;
											else answer[i]=65+com;
									}
									answer[i]=NULL;
									FadeOut();

									if(strcmp(answer,"NOTHING")==0)
									{
										m_dlg.TextSnr(175);
										int m_4w1h[5];
										for(int i=0; i<5; i++)m_4w1h[i]=m_dlg.TextSel(177+i);

										if(m_4w1h[0]==4 && m_4w1h[1]==1 && m_4w1h[2]==2 && m_4w1h[3]==9 && m_4w1h[4]==0)
										{
											m_dlg.TextSnr(182);
										}
										else m_dlg.TextSnr(176);
									}
									else
									{
										m_dlg.TextPut(StrAdd("ccc","27999",answer,"���?\\��... �׷���.\\�ƹ�ư �����ߴ�."));
										m_dlg.TextPut(StrAdd("ccc","00999�׷��� ",answer,"�� ���̶�� �����ϰ� ��� ��Ҵ�."));
									}
									FadeOut();
									_MidiPlay(bgm[10]);
									m_dlg.TextSnr(183,BACK_ENDING);
									m_sv.sw[SW_QUIT]=true;
									FadeOut();
								}
							}							
							else if(m_sv.sw[SW_VISIT_MAX])	//ǳ���� ����
							{
								m_dlg.TextSnr(3);
								m_sv.sw[SW_GET_BALLOON]=true;
								m_sv.var[VAR_AIR]=1000;
							}
							else
							{
								m_dlg.TextSnr(2);
							}
						}
						else if(cx==6 && cy==1)	//����
						{
							if(m_sv.sw[SW_GET_BALLOON])	//ǳ�� ��� ����
							{
								AirPort();
							}
							else	//���� �� ����
							{
								m_dlg.TextSnr(6);
							}
						}
					}
					////////////////////////////////////////////////
					//������ ����
					else if(m_sv.var[VAR_SPOT]==1)
					{
						if(cx==4 && cy==1)//�̾� ��
						{
							NormalHouse(5);
						}
						else if(cx==5 && cy==1)//���� ��
						{
							NormalHouse(4);
						}
						else if(cx==4 && cy==2)//��� ��
						{
							NormalHouse(6);
						}
						else if(cx==2 && cy==2)//������ ��
						{
							NormalHouse(11);
						}
						else if(cx==7 && cy==1)//���� ����
						{
							NormalHouse(29);
						}
						else if(cx==4 && cy==3)//���̴� ��
						{
							if(!m_sv.sw[SW_FIRST_FAINY])
							{
								m_dlg.TextSnr(28);
								m_sv.sw[SW_FIRST_FAINY]=true;
							}

							//�¾��� �ð�
							if(m_sv.sw[SW_ITEM+2] && m_sv.sw[SW_ITEM+7] && m_sv.sw[SW_SET_SEED] &&
								m_sv.var[VAR_TIME+3]>=12 && m_sv.var[VAR_TIME+3]<=14)
							{
								m_dlg.TextSnr(148);
								WhiteOut();
								m_dlg.TextSnr(149);
								_Play(11);
								m_dlg.TextSnr(150);
								m_sv.sw[SW_ITEM+2]=false;
								m_sv.sw[SW_ITEM+7]=false;
								m_sv.sw[SW_RAIN]=true;
								m_sv.var[VAR_SOLUTION]=5;
							}
							else
							{
								int com=-1;
								while(com!=0)
								{
									CCommand m_liblary(6);
									m_liblary.AddComs(FOUR,1,2,3,0);
									com=m_liblary.GetCommand();
									if(com>0)m_dlg.TextSnr(118+com);
								}
							}
						}
						else if(cx==6 && cy==2)//����
						{
							if(!m_party.IsIt(18) && m_sv.sw[SW_RAIN])
							{
								m_dlg.TextSnr(151);
								m_party.Insert(18);
								m_sv.var[VAR_PARTY_ADD]++;
							}
							AirPort();
						}
					}
					////////////////////////////////////////////////
					//å ����
					else if(m_sv.var[VAR_SPOT]==2)
					{
						if(cx==3 && cy==2)//������ ��
						{
							NormalHouse(19);
						}
						else if(cx==1 && cy==2)//����� ��
						{
							if(!m_party.IsIt(12))
							{
								//��� ���ᰡ ��
								m_dlg.TextSnr(29);
								m_party.Insert(12);
								m_sv.var[VAR_PARTY_ADD]++;
							}
							else m_dlg.TextSnr(32);
						}
						else if(cx==5 && cy==2)//������
						{
							if(m_sv.sw[SW_ITEM+5])
							{
								m_dlg.TextSnr(114);
								int decision=m_dlg.TextSel(108);
								if(decision==0)
								{
									m_dlg.TextSnr(117);
									m_sv.var[VAR_MONEY]+=1000;
									m_sv.sw[SW_ITEM+5]=false;
								}
							}

							m_dlg.TextSnr(33);
							CCommand m_com(0);
							m_com.AddComs(THREE,1,4,0);
							int com=m_com.GetCommand();
							if(com==1)
							{
								m_dlg.TextSnr(52);
							}
							if(com==4)
							{
								//��Ģ ����
								if(!m_sv.sw[SW_GAMBLE])
								{
									m_dlg.TextSnr(53);
									m_sv.sw[SW_GAMBLE]=true;
								}
								//���� ����
								int money_level=1;
								_MidiPlay(bgm[7]);
								while(1)
								{
									if(m_sv.var[VAR_MONEY]<money_level)
									{
										m_dlg.TextSnr(54);
										break;
									}
									m_dlg.TextPut(StrAdd("cdc","30000",money_level,"ī���� �ɰ� �����̴���."));
									int victory=Gamble();
									int all_money=money_level*victory;
									//�й�
									if(victory<0)
									{
										m_dlg.TextPut(StrAdd("cdc","30000",abs(all_money),"ī�� ����������."));
										m_sv.var[VAR_MONEY]+=all_money;
										m_dlg.TextSnr(56);
										break;
									}
									//�¸�
									else if(victory>=0)
									{
										if(victory>0)
										{
											m_dlg.TextPut(StrAdd("ccdc","30000","���� ������.",abs(all_money),"ī�� ����������."));
											m_sv.var[VAR_MONEY]+=all_money;
										}
										m_dlg.TextSnr(55);
										int yn=m_dlg.TextSel("�Ѵ�.\\�׸� �д�.");
										if(yn==0)
										{
											if(money_level<100000000)money_level*=10;	//���Ѽ� 1��
										}
										else break;
									}
								}
								_MidiPlay(bgm[m_sv.var[VAR_SPOT]]);
							}
						}
						else if(cx==4 && cy==1)//������ ��
						{
							if(TRIBE_SKY)
							{
								bool use_library=true;
								//���� ����
								if(m_sv.sw[SW_ITEM])
								{
									m_dlg.TextSnr(14);
									int decision=m_dlg.TextSel(17);
									m_sv.sw[SW_ITEM]=false;
									m_sv.sw[SW_SET_SEED]=true;
									if(decision==0)m_dlg.TextSnr(19);
									else
									{
										m_dlg.TextSnr(103);
										m_sv.sw[SW_SEED_MONEY]=true;
										m_sv.var[VAR_MONEY]+=30000;										
									}
								}
								else
								{
									if(m_sv.sw[SW_END_OCT])
									{
										m_dlg.TextSnr(35);
									}
									//�ູ�� ��
									else if(m_sv.sw[SW_ITEM+8])
									{
										if(m_sv.sw[SW_ATTACK])
										{
											bool fightable=false;
											for(int i=1; i<=10; i++)
												if(m_party.IsIt(i))
												{
													fightable=true;
													break;
												}
											int vs, enemy;
											if(fightable)
											{
												m_dlg.TextSnr(169);
												//�ο� �� �ִ� ��� Ȯ��
												CCommand m_our_fighter(3);
												for(int i=1; i<=10; i++)
													if(m_party.IsIt(i))m_our_fighter.AddCom(i-1);

												//���ı��� ����
												if(m_sv.sw[SW_RE_PARK])
												{
													m_dlg.TextSnr(170);
													enemy=7;
													vs=0;
													while(vs==0)
													{
														m_dlg.TextSnr(135);
														int army=m_our_fighter.GetCommand()+1;
														vs=Fight(army-1,enemy-1);
														if(vs==-1)m_sv.sw[SW_QUIT]=true;
													}
												}

												//�¸����� ��
												if(!m_sv.sw[SW_QUIT])
												{
													//������ ����
													if(m_sv.sw[SW_RE_HELL])
													{
														m_dlg.TextSnr(171);
														enemy=8;
														vs=0;
														while(vs==0)
														{
															m_dlg.TextSnr(135);
															int army=m_our_fighter.GetCommand()+1;
															vs=Fight(army-1,enemy-1);
															if(vs==-1)m_sv.sw[SW_QUIT]=true;
														}
													}
												}

												//�¸����� ��
												if(!m_sv.sw[SW_QUIT])
												{
													//�������� �Ƶ��
													m_dlg.TextSnr(172);
													enemy=1;
													vs=0;
													while(vs==0)
													{
														m_dlg.TextSnr(135);
														int army=m_our_fighter.GetCommand()+1;
														vs=Fight(army-1,enemy-1);
														if(vs==-1)m_sv.sw[SW_QUIT]=true;
													}
												}

												//�¸�
												if(!m_sv.sw[SW_QUIT])
												{
													m_dlg.TextSnr(173);
													m_sv.sw[SW_END_OCT]=true;
													m_sv.var[VAR_SOLUTION]=8;
												}
											}
											else m_dlg.TextSnr(168);
											use_library=false;
										}
										else m_dlg.TextSnr(158);
									}
									//������ �Ȱ�
									else if(m_sv.sw[SW_FIND_TRUE])
									{
										if(m_sv.sw[SW_ITEM+1] && m_sv.sw[SW_ITEM+4])
										{
											m_dlg.TextSnr(155);
											MakeGlass();
										}
										else m_dlg.TextSnr(159);
									}
									//���ѿ� ���õ� ��
									else if(m_sv.sw[SW_SET_SEED])
									{
										if(m_sv.sw[SW_RAIN])
										{
											m_dlg.TextSnr(153);
											m_sv.sw[SW_FIND_TRUE]=true;
											m_sv.var[VAR_SOLUTION]=7;
											if(m_sv.sw[SW_ITEM+1] && m_sv.sw[SW_ITEM+4])
											{
												m_dlg.TextSnr(154);
												MakeGlass();
											}
										}
										else m_dlg.TextSnr(104);
									}									
									else m_dlg.TextSnr(35);	//���� ��

									if(use_library)
									{
										int com=-1;
										while(com!=0)
										{
											CCommand m_liblary(6);
											m_liblary.AddComs(FOUR,4,5,6,0);
											com=m_liblary.GetCommand();
											if(com>0)m_dlg.TextSnr(118+com);
										}
									}
								}
							}
							else m_dlg.TextSnr(34);
						}
						else if(cx==8 && cy==1)//����� ��
						{
							NormalHouse(13);
						}
						else if(cx==4 && cy==2)//����
						{
							//���ı��� ������
							if(!m_sv.sw[SW_GANG_PARK] && m_sv.sw[SW_GANG]
								&& m_sv.var[VAR_MONEY]>=1000 &&
								m_sv.var[VAR_TIME+3]>=9 && m_sv.var[VAR_TIME+3]<=17)
							{
								m_sv.sw[SW_GANG]=false;
								m_dlg.TextSnr(128);
								CCommand m_gang(7);
								for(int i=1; i<=10; i++)
									if(m_party.IsIt(i))
									{
										m_gang.AddCom(1);
										break;
									}
								m_gang.AddCom(0);

								int com=m_gang.GetCommand();
								if(com==0)
								{
									m_sv.var[VAR_MONEY]-=1000;
									m_dlg.TextSnr(129);
								}
								else
								{
									int enemy=7;
									CCommand m_our_fighter(3);
									for(int i=1; i<=10; i++)
										if(m_party.IsIt(i))m_our_fighter.AddCom(i-1);
									m_dlg.TextSnr(135);
									int army=m_our_fighter.GetCommand()+1;
									int vs=Fight(army-1,enemy-1);
									if(vs==-1)	//�й�
									{
										m_sv.var[VAR_MONEY]-=1000;
										m_dlg.TextSnr(134);
									}
									else if(vs==1)
									{
										m_sv.sw[SW_GANG_PARK]=true;
										CCommand m_win(8);
										m_win.AddComs(TWO,0,2);
										if(m_sv.var[VAR_PARTY_NUM]<8+m_sv.var[VAR_PARTY_ADD])m_win.AddCom(1);
										
										m_dlg.TextSnr(130);
										com=m_win.GetCommand();
										//ó��
										if(com==0)
										{
											_Play(0);
											m_dlg.TextSnr(131);
											int rank_kill=GetRank(enemy);
											for(int i=rank_kill; i<9; i++)
												m_sv.var[VAR_RANK+i]=m_sv.var[VAR_RANK+i+1];
											m_sv.var[VAR_RANK+9]=enemy;
											m_sv.var[VAR_DEATH]++;
										}
										//����
										else if(com==1)
										{
											m_dlg.TextSnr(132);
											m_party.Insert(enemy);
										}
										//�� ���
										else if(com==2)
										{
											m_sv.sw[SW_RE_PARK]=true;
											m_dlg.TextSnr(133);
											m_sv.var[VAR_MONEY]+=10000;
										}
									}
									else m_dlg.TextSnr(133);
								}
							}
							else AirPort();
						}
					}
					////////////////////////////////////////////////
					//�� ����
					else if(m_sv.var[VAR_SPOT]==3)
					{
						if(cx==4 && cy==2)//������ ��
						{
							NormalHouse(9);
						}
						else if(cx==4 && cy==3)//������ ��
						{
							NormalHouse(10);
						}
						else if(cx==6 && cy==2)//������ ��
						{
							NormalHouse(14);
						}
						else if(cx==1 && cy==2)//������ ��
						{
							NormalHouse(24);
						}
						else if(cx==5 && cy==1)//������ ��
						{
							NormalHouse(31);
						}
						else if(cx==5 && cy==4)//����
						{
							//������ ������
							if(!m_sv.sw[SW_GANG_HELL] && m_sv.sw[SW_GANG] && (TRIBE_SKY) &&
								m_sv.var[VAR_MONEY]>=1000 &&
								(m_sv.var[VAR_TIME+3]>=17 || m_sv.var[VAR_TIME+3]<=2))
							{
								m_sv.sw[SW_GANG]=false;
								m_dlg.TextSnr(136);
								CCommand m_gang(7);
								for(int i=1; i<=10; i++)
									if(m_party.IsIt(i))
									{
										m_gang.AddCom(1);
										break;
									}
								m_gang.AddCom(0);

								int com=m_gang.GetCommand();
								if(com==0)
								{
									m_sv.var[VAR_MONEY]-=1000;
									m_dlg.TextSnr(137);
								}
								else
								{
									int enemy=8;
									CCommand m_our_fighter(3);
									for(int i=1; i<=10; i++)
										if(m_party.IsIt(i))m_our_fighter.AddCom(i-1);
									m_dlg.TextSnr(135);
									int army=m_our_fighter.GetCommand()+1;
									int vs=Fight(army-1,enemy-1);
									if(vs==-1)	//�й�
									{
										m_sv.var[VAR_MONEY]-=1000;
										m_dlg.TextSnr(142);
									}
									else if(vs==1)
									{
										m_sv.sw[SW_GANG_HELL]=true;
										CCommand m_win(8);
										m_win.AddComs(TWO,0,2);
										if(m_sv.var[VAR_PARTY_NUM]<8+m_sv.var[VAR_PARTY_ADD])m_win.AddCom(1);
										
										m_dlg.TextSnr(138);
										com=m_win.GetCommand();
										//ó��
										if(com==0)
										{
											_Play(1);
											m_dlg.TextSnr(139);
											int rank_kill=GetRank(enemy);
											for(int i=rank_kill; i<9; i++)
												m_sv.var[VAR_RANK+i]=m_sv.var[VAR_RANK+i+1];
											m_sv.var[VAR_RANK+9]=enemy;
											m_sv.var[VAR_DEATH]++;
										}
										//����
										else if(com==1)
										{
											m_dlg.TextSnr(140);
											m_party.Insert(enemy);
										}
										//�� ���
										else if(com==2)
										{
											m_sv.sw[SW_RE_HELL]=true;
											m_dlg.TextSnr(141);
											m_sv.var[VAR_MONEY]+=10000;
										}
									}
									else m_dlg.TextSnr(141);
								}
							}
							else AirPort();
						}
					}
					////////////////////////////////////////////////
					//�� ����
					else if(m_sv.var[VAR_SPOT]==4)
					{
						if(cx==4 && cy==3)	//Ŭ��� ��
						{
							NormalHouse(2);
						}
						else if(cx==5 && cy==1)//�������� ��
						{
							NormalHouse(3);
						}
						else if(cx==4 && cy==1)//������ ��
						{
							NormalHouse(15);
						}
						else if(cx==9 && cy==4)//���̾��� ��
						{
							NormalHouse(23);
						}
						else if(cx==6 && cy==1)//����
						{
							AirPort();
						}
					}
					////////////////////////////////////////////////
					//������ ����
					else if(m_sv.var[VAR_SPOT]==5)
					{
						if(cx==5 && cy==0)	//������ ���� ����
						{
							CCommand m_liblary(6);
							m_liblary.AddComs(TWO,7,0);
							int com=m_liblary.GetCommand();
							if(com>0)m_dlg.TextSnr(118+com);
						}
						else if(cx==3 && cy==2)//���� ����
						{
							if(!m_sv.sw[SW_MONEY_BOX])
							{
								m_dlg.TextSnr(161);
								m_sv.var[VAR_MONEY]+=100000;
								m_sv.sw[SW_MONEY_BOX]=true;
							}
							else m_dlg.TextSnr(164);
						}
						else if(cx==6 && cy==2)//���� ����
						{
							if(!m_sv.sw[SW_MONEY_BOX])
							{
								m_dlg.TextSnr(162);
								m_sv.var[VAR_MONEY]+=50000;
								m_sv.sw[SW_MONEY_BOX]=true;
							}
							else m_dlg.TextSnr(164);
						}
						else if(cx==7 && cy==2)//���� ����
						{
							if(!m_sv.sw[SW_MONEY_BOX])
							{
								m_dlg.TextSnr(163);
								m_sv.var[VAR_MONEY]+=30000;
								m_sv.sw[SW_MONEY_BOX]=true;
							}
							else m_dlg.TextSnr(164);
						}
						else if(cx==4 && cy==3)//�ູ�� ��
						{
							if(!m_sv.sw[SW_GET_FIRE])
							{
								m_dlg.TextSnr(165);
								m_sv.sw[SW_ITEM+6]=true;
								m_sv.sw[SW_GET_FIRE]=true;
							}
							else m_dlg.TextSnr(164);
						}
						else if(cx==5 && cy==2) //������ ��
						{
							if(m_sv.sw[SW_ITEM+8])
							{
								//�ٸ� ���� ����
								if(!m_sv.sw[SW_ATTACK] && m_sv.sw[SW_GET_FIRE])
								{
									m_dlg.TextSnr(166);
									FadeOut();
									m_dlg.TextSnr(167);
									m_sv.sw[SW_ITEM+6]=false;
									m_sv.sw[SW_ATTACK]=true;
								}

								FadeOut();
								m_sv.var[VAR_SPOT]=0;
								_MidiPlay(bgm[m_sv.var[VAR_SPOT]]);
							}
						}
					}
				}
			}
			else enter=false;
			jdd->DrawPicture(SCREEN_BUFFER,StrAdd("cd","Map",m_sv.var[VAR_SPOT]),0,0,NULL);
			if(m_sv.var[VAR_SPOT]==0 && m_sv.sw[SW_ITEM+8])jdd->DrawPicture(SCREEN_BUFFER,"Hole",32*5,32*3,NULL);
			jdd->DrawPicture(SCREEN_BUFFER,"Cursor",cx*32+16,cy*32+16,NULL);
			jdd->DrawText(SCREEN_BUFFER,StrAdd("dcdcdcdc",m_sv.var[VAR_TIME],"�� ",m_sv.var[VAR_TIME+1],"�� ",
				m_sv.var[VAR_TIME+2],"�� ",m_sv.var[VAR_TIME+3],"��"),font12,1,1,JColor(0,0,0));
			jdd->DrawText(SCREEN_BUFFER,global_buffer,font12,0,0,JColor(255,128,0));
			int put_money=m_sv.var[VAR_MONEY];
			int px=276;
			while(put_money>=10)
			{
				put_money/=10;
				px-=11;
			}

			jdd->DrawText(SCREEN_BUFFER,StrAdd("dc",m_sv.var[VAR_MONEY],"ī��"),font12,px+1,1,JColor(0,0,0));
			jdd->DrawText(SCREEN_BUFFER,StrAdd("dc",m_sv.var[VAR_MONEY],"ī��"),font12,px,0,JColor(196,196,0));
			Render();
			
			//�޴�
			if(_GetKeyState(VK_ESCAPE))
			{
				if(!esc)
				{
					esc=true;

					FadeOut();
					int result=m_game.Menu();
					if(result==999)	//������
					{
						m_sv.sw[SW_QUIT]=true;
					}
					FadeOut();
				}
			}
			else esc=false;
		}

		//���θ޴�
		_MidiPlay(bgm[6]);
		jdd->DrawPicture(SCREEN_BUFFER,"Title",0,0,NULL);
		Render();
		Sleep(1000);
		while(m_sv.sw[SW_QUIT])
		{
			int main_menu=m_dlg.TextSel(15);
			//�� ����
			if(main_menu==0)
			{
				//�ʱ�ȭ
				m_sv.InitSwitch(false);
				m_sv.InitVar(0);

				m_sv.SetVar(VAR_SPOT,VAR_MONEY,0);
				m_sv.SetVars(VAR_RANK,VAR_RANK+9,1,2,3,4,5,6,7,8,9,10);	//���� ��ŷ
				//���� �ó�����
				jdd->DrawPicture(SCREEN_BUFFER,"Map0",0,0,NULL);
				m_dlg.TextSnr(0);
				//�����ϴ� ��: 54�� 5�� 1�� 7��
				m_sv.SetVars(VAR_TIME,VAR_TIME+3,54,5,1,7);
				//������ �ɷ�
				m_sv.SetVars(VAR_HP,VAR_HP+9,350,280,320,300,170,200,150,80,120,100);
				m_sv.SetVars(VAR_POW,VAR_POW+9,5,5,5,3,3,2,3,3,1,1);
				m_sv.SetVars(VAR_SPD,VAR_SPD+9,5,5,3,3,5,2,2,3,2,1);
				m_sv.sw[SW_QUIT]=false;
			}
			//�ҷ�����
			else if(main_menu==1)
			{
				char savefiles[256];
				char savebuffer[64];
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
						wsprintf(savebuffer,"%d��%d��%d��%d�� %s����",save_time[0],save_time[1],save_time[2],save_time[3],cloud_name[save_spot]);
						strcat(savefiles,savebuffer);
						fclose(fp);
					}
					else strcat(savefiles,"��� ����");
					strcat(savefiles,"\\");
				}
				strcat(savefiles,"���");

				//����
				while(1)
				{
					int no=m_dlg.TextSel(savefiles,-1,load_point);
					if(no<FILE_MAX)
					{
						if(m_game.Load(no+1))
						{
							m_sv.sw[SW_QUIT]=false;
							break;
						}
						else load_point=no;
					}
					else break;
				}
			}
			//������
			else if(main_menu==2)
			{
				end=true;
				m_sv.sw[SW_QUIT]=false;
			}
		}
		FadeOut();
		if(!end)_MidiPlay(bgm[m_sv.var[VAR_SPOT]]);
	}

	//���� ��, ����
	jdd->DeleteFont(font12);
	jdd->DeleteFont(font20);
	jdd->DeleteFont(font10);
	jdd->DeleteFont(font32);
	delete jdd;

	_CrtDumpMemoryLeaks();

	return 0;
}

LRESULT CALLBACK WndProc(HWND wnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
    switch ( msg )
    {
		case MM_MCINOTIFY    :  if ( ReplayFlag && wParam == MCI_NOTIFY_SUCCESSFUL ) _MidiReplay();
								break;

		case WM_MOVE		 :	if(jdd)jdd->OnMove(LOWORD(lParam), HIWORD(lParam));
								break;
		
		case WM_SIZE		 :	if(wParam == SIZE_MINIMIZED)activate=false;
								else activate=true;
								break;
		
		case WM_ACTIVATE	 : if(LOWORD(wParam))activate=true;
								else activate=false;
							   break;

		case WM_SYSCOMMAND	 :  //�ݱ� �޽��� ����ä��
								if(wParam==SC_CLOSE)
								{
									wParam=0;
									exit(0);
								}
								break;

	}

	return DefWindowProc(wnd,msg,wParam,lParam);
}