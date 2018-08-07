/**********************************************************************************************
 * Arduino PID Library - Version 1.1.1
 * by Brett Beauregard <br3ttb@gmail.com> brettbeauregard.com
 * This Library is licensed under a GPLv3 License
 **********************************************************************************************/
#include "pid_pv1.h"
#include <rtthread.h>
/*Constructor (...)*********************************************************
 *    The parameters specified here are those for for which we can't set up 
 *    reliable defaults, so we need to have the user set them.
 ***************************************************************************/
PID::PID(double* Input, double* Output, double* Setpoint,
        double Kp, double Ki, double Kd, int ControllerDirection)
{
	// ��ֵ�����������������趨ֵ��ʼ��ַ��ע�������ǵ�ַ
    myOutput = Output;
    myInput = Input;
    mySetpoint = Setpoint;
	// ��ʼ��autoģʽΪfalse
	inAuto = false;
	
	// Ĭ�Ͽ�����������0��255���˺������Ը���ʵ��ϵͳ��Ҫ�޸Ŀ�����������Ʒ�Χ
	PID::SetOutputLimits(0, 255);				//default output limit corresponds to 
												//the arduino pwm limits
	// Ĭ�ϲ�������Ϊ100ms��ͬ�����Ը��������޸�
    SampleTime = 100;							//default Controller Sample Time is 0.1 seconds

	// ��������ķ���
    PID::SetControllerDirection(ControllerDirection);
	// ����PID ���Ʋ���
    PID::SetTunings(Kp, Ki, Kd);

	// ���ڴ洢PID����ʱ����Ӧ��ϵͳ����ʱ��
	// rt_tick_get()�����ǻ�ȡ��ǰϵͳ����ʱ�䣨��λms�����˺������arduino����ֲ�����ϵͳ�����������������ú������
	// �����ȥSampleTime��Ϊ�˱�֤�ڹ�����������Ͻ���PID���ƣ�������Ҫ�ȴ�����һ��SampleTime����
    lastTime = rt_tick_get()-SampleTime;				
}
 
 
/* Compute() **********************************************************************
 *     This, as they say, is where the magic happens.  this function should be called
 *   every time "void loop()" executes.  the function will decide for itself whether a new
 *   pid Output needs to be computed.  returns true when the output is computed,
 *   false when nothing has been done.
 *   �˺�������PID���������㣬��������Ƶ�����ڽ����б����á�
 **********************************************************************************/ 
bool PID::Compute()
{
	// ���û�п���PID���� ����ʧ�ܣ��˳������������䣬��Ϊ��һ�ο�����
   if(!inAuto) return false;
   // ��ȡ��ǰϵͳ����ʱ�䲢��������һ�μ���ʱ����
   unsigned long now = rt_tick_get();
   unsigned long timeChange = (now - lastTime);
   // ���ʱ�������ڻ��ߵ��ڲ���ʱ�䣬��ô�����,���������������������ʧ�ܣ��˳���
   if(timeChange>=SampleTime)
   {
      /*Compute all the working error variables*/
	   // ���浱ǰ�������������һ��ʵʱ����ϵͳ����ʱ�����������빹��ʱ�ı�������һ��
	  double input = *myInput;
	  // ����趨ֵ�뵱ǰ������֮���ƫ��
      double error = *mySetpoint - input;
	  // ��������� �˴�������ͱ�׼PID���Ʒ�����΢�в��
      ITerm+= (ki * error);
	  // ��� �������������ƣ���ô���û�����Ϊ������ƣ�ͬ������С����Ҳ��ͬ������
	  // �˴�Ϊ����ô��һ������˵���������Ҫ��Ϊ��PID ��������ʱ�䳬�޺�ͻȻ�����趨ֵ���ܹ���ϵͳ���Ϸ�Ӧ���������һ��ʱ���ͺ�
      if(ITerm > outMax) ITerm= outMax;
      else if(ITerm < outMin) ITerm= outMin;

	  // �������������֮��ƫ�Ҳ�����ڼ������ڣ����ﲻ�ò�����������Ϊ�������ڿ��ܻᳬ���������ڣ��������ı仯��
	  // ��ʵ����΢����� ���ӣ����ǿ������ͱ�׼���ʽҲ��һ����������
	  // ��������һ����Ҳ˵��������ܵ���˵��Ϊ�˷�ֹ�������ͱ�����ͻ��
      double dInput = (input - lastInput);
 
      /*Compute PID Output*/
	  // PID ������ʽ����Ͳ���Ҫ˵����
      double output = kp * error + ITerm- kd * dInput;

      // ���������ƺ�ITerm�����Ƶ�������һ���ġ���
	  if(output > outMax) output = outMax;
      else if(output < outMin) output = outMin;
	  *myOutput = output;
	  
      /*Remember some variables for next time*/
      lastInput = input;
      lastTime = now;
	  return true;
   }
   else return false;
}


/* SetTunings(...)*************************************************************
 * This function allows the controller's dynamic performance to be adjusted. 
 * it's called automatically from the constructor, but tunings can also
 * be adjusted on the fly during normal operation
 * �˺��������趨PID���ڲ���
 ******************************************************************************/ 
void PID::SetTunings(double Kp, double Ki, double Kd)
{
	// ���PID��������С��0�Ĳ�������ô�趨ʧ�ܣ�ֱ���˳�����Ȼ����ԭ���Ĳ���
   if (Kp<0 || Ki<0 || Kd<0) return;
	// ������ʾ�á�
   dispKp = Kp; dispKi = Ki; dispKd = Kd;
   
   // ��ȡ����ʱ�䣬��msתΪs
   double SampleTimeInSec = ((double)SampleTime)/1000;  
   // ����PID������ I �� D �����ĵ�����Ҫ��Ϊ������������ڸı�����µ�Ӱ�죬
   // ��Ҫ�� ������� ΢�����Ǻ�ʱ���йصĲ��������Բ������ڸı�ᵼ����������Ҫ���¼��㣬����Ϊ�˼�����Щ���������������ڱ任ת����I D�����仯
   // ����Ϊʲô������ô��������Ϊǰ���������⴦���޸���PID��׼���ʽ��ʹÿһ�μ������ʷ������С
   kp = Kp;
   ki = Ki * SampleTimeInSec;
   kd = Kd / SampleTimeInSec;
 
	//  �趨PID���ڷ���
  if(controllerDirection ==REVERSE)
   {
      kp = (0 - kp);
      ki = (0 - ki);
      kd = (0 - kd);
   }
}
  
/* SetSampleTime(...) *********************************************************
 * sets the period, in Milliseconds, at which the calculation is performed	
 ******************************************************************************/
//�����µĲ���ʱ�䣬ͬʱ���ձ�������ID����
void PID::SetSampleTime(int NewSampleTime)
{
   if (NewSampleTime > 0)
   {
      double ratio  = (double)NewSampleTime
                      / (double)SampleTime;
      ki *= ratio;
      kd /= ratio;
      SampleTime = (unsigned long)NewSampleTime;
   }
}
 
/* SetOutputLimits(...)****************************************************
 *     This function will be used far more often than SetInputLimits.  while
 *  the input to the controller will generally be in the 0-1023 range (which is
 *  the default already,)  the output will be a little different.  maybe they'll
 *  be doing a time window and will need 0-8000 or something.  or maybe they'll
 *  want to clamp it from 0-125.  who knows.  at any rate, that can all be done
 *  here.
 * �˺������ײ�����������ͻ�䣬�����й����У�������Ҫ��С��Χ
 **************************************************************************/
void PID::SetOutputLimits(double Min, double Max)
{
	// ��ֵ����
   if(Min >= Max) return;
   outMin = Min;
   outMax = Max;
 
   if(inAuto)
   {
	   if(*myOutput > outMax) *myOutput = outMax;
	   else if(*myOutput < outMin) *myOutput = outMin;
	 
	   if(ITerm > outMax) ITerm= outMax;
	   else if(ITerm < outMin) ITerm= outMin;
   }
}

/* SetMode(...)****************************************************************
 * Allows the controller Mode to be set to manual (0) or Automatic (non-zero)
 * when the transition from manual to auto occurs, the controller is
 * automatically initialized
 ******************************************************************************/ 
void PID::SetMode(int Mode)
{
    bool newAuto = (Mode == AUTOMATIC);
	// ���ģʽ��һ������ô�����³�ʼ��
    if(newAuto == !inAuto)
    {  /*we just went from manual to auto*/
        PID::Initialize();
    }
    inAuto = newAuto;
}
 
/* Initialize()****************************************************************
 *	does all the things that need to happen to ensure a bumpless transfer
 *  from manual to automatic mode.
 ******************************************************************************/ 
void PID::Initialize()
{
   ITerm = *myOutput;
   lastInput = *myInput;
   if(ITerm > outMax) ITerm = outMax;
   else if(ITerm < outMin) ITerm = outMin;
}

/* SetControllerDirection(...)*************************************************
 * The PID will either be connected to a DIRECT acting process (+Output leads 
 * to +Input) or a REVERSE acting process(+Output leads to -Input.)  we need to
 * know which one, because otherwise we may increase the output when we should
 * be decreasing.  This is called from the constructor.
 ******************************************************************************/
void PID::SetControllerDirection(int Direction)
{
   if(inAuto && Direction !=controllerDirection)
   {
	  kp = (0 - kp);
      ki = (0 - ki);
      kd = (0 - kd);
   }   
   controllerDirection = Direction;
}

/* Status Funcions*************************************************************
 * Just because you set the Kp=-1 doesn't mean it actually happened.  these
 * functions query the internal state of the PID.  they're here for display 
 * purposes.  this are the functions the PID Front-end uses for example
 ******************************************************************************/
double PID::GetKp(){ return  dispKp; }
double PID::GetKi(){ return  dispKi;}
double PID::GetKd(){ return  dispKd;}
int PID::GetMode(){ return  inAuto ? AUTOMATIC : MANUAL;}
int PID::GetDirection(){ return controllerDirection;}

