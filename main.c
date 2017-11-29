/*
 * main.c
 *
 *  Created on: 2017��11��27��
 *      Author: Wiiboox
 */
#include "CAN_TEST.h"
#include "ObjDict.h"
#include "Can.h"

Uint64 numb;
Uint32 datatmpl;
Uint32 datatmph;
Uint32 CanId = 1;

static Message m = Message_Initializer;		// contain a CAN message
void CanTest2()
{
	if (flag == 1)
	{
		CanTxMsg(canrxmsg);
		flag = 0;
	}
}
void CanTxTest(void)
{
	Uint32 datatmpl = 0, datatmph = 0;
	MessageExt msg;

	datatmph = numb >> 32;
	datatmpl = numb & 0xFFFFFFFF;
    //extend frame
	msg.cob_id = (Uint32)0x80000000 | CanId;
	msg.rtr = 0;
	msg.len = 8;
	msg.data[0] = datatmpl & 0xFF;
	msg.data[1] = (datatmpl >> 8) & 0xFF;
	msg.data[2] = (datatmpl >> 16) & 0xFF;
	msg.data[3] = (datatmpl >> 24) & 0xFF;
	msg.data[4] = datatmph & 0xFF;
	msg.data[5] = (datatmph >> 8) & 0xFF;
	msg.data[6] = (datatmph >> 16) & 0xFF;
	msg.data[7] = (datatmph >> 24) & 0xFF;
    //senddata
	if (CanTxMsg(msg) > 0)
	{
		numb++;
		CanId++;
		if (CanId > (Uint32)0x1FFFFFFF)
		{
			CanId = 0;
		}
	}

	if (canastate == BUSOFF)
	{
		InitECan();
		canastate = WORKED;
	}
}



void main(void)
{

	numb = 1;
	CanId = 1;
	datatmpl = 0;
	datatmph = 1;
	canastate = IDEL;
	/*��ʼ��ϵͳ*/
	InitSysCtrl();

	/*���ж�*/
	DINT;
	IER = 0x0000;
	IFR = 0x0000;

	/*��ʼ��PIE�ж�*/
	InitPieCtrl();

	/*��ʼ��PIE�ж�ʸ����*/
	InitPieVectTable();
	/*time1 for interrupt*/
	EALLOW;
	PieVectTable.TINT0 = &cpu_timer0_isr;
	EDIS;
	InitCpuTimers();
    ConfigCpuTimer(&CpuTimer0, 150, 1000);
	CpuTimer0Regs.TCR.all = 0x4001;
	/*��ʼ��SCIA�Ĵ���*/
    InitECan();
    canastate = INITED;
    flag = 0;
    //ʹ��PIE�ж�
    PieCtrlRegs.PIEIER9.bit.INTx5 = 1;
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
	//ʹ��CPU�ж�
	IER |= M_INT9;
	IER |= M_INT1;
	//PieCtrlRegs.PIEIER1.bit.INTx7 = 1;

	EINT;   //��ȫ���ж�
	ERTM;	//��ʵʱ�ж�
	canastate = WORKED;

	setNodeId (&ObjDict_Data, CanId);
	setState(&ObjDict_Data, Initialisation);	// Init the state
    for(;;)
	{
        if (canReceive(&m))			// a message reveived
            canDispatch(&ObjDict_Data, &m);         // process it
	}

}

