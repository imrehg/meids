package de.meilhaus.medriver;

public class MeStatus{
	private int iStatus;
	private int iCount;

	public MeStatus(){
		iStatus = 0;
		iCount = 0;
	}

	public int getStatus(){
		return this.iStatus;
	}
	public int getCount(){
		return this.iCount;
	}

	public void setStatus(int iStatus){
		this.iStatus = iStatus;
	}
	public void setCount(int iCount){
		this.iCount = iCount;
	}
}
