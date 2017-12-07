#ifndef RUNNING_SUM_H_
#define RUNNING_SUM_H_

class RunningSum
{
public:
    RunningSum(float decayConstant) :
        decayConstant(decayConstant), runningSum(0)
    {

    }
    
    float UpdateSum(float x)
    {
        runningSum *= decayConstant;
        runningSum += x;
    	return runningSum * (1 - decayConstant);
    }
    
    void Clear()
    {
    	runningSum = 0;
    }

private:
    float decayConstant;
    float runningSum;
};

#endif
