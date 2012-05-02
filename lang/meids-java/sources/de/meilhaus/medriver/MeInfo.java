package de.meilhaus.medriver;

public class MeInfo{
	private int iVendorId;
	private int iDeviceId;
	private int iSerialNo;
	private int iBusType;
	private int iBusNo;
	private int iDevNo;
	private int iFuncNo;
	private int iPlugged;

	public MeInfo(){
		iVendorId = 0;
		iDeviceId = 0;
		iSerialNo = 0;
		iBusType = 0;
		iBusNo = 0;
		iDevNo = 0;
		iFuncNo = 0;
		iPlugged = 0;
	}

	public void setVendorId(int iVendorId){
		this.iVendorId = iVendorId;
	}
	public void setDeviceId(int iDeviceId){
		this.iDeviceId = iDeviceId;
	}
	public void setSerialNo(int iSerialNo){
		this.iSerialNo = iSerialNo;
	}
	public void setBusType(int iBusType){
		this.iBusType = iBusType;
	}
	public void setBusNo(int iBusNo){
		this.iBusNo = iBusNo;
	}
	public void setDevNo(int iDevNo){
		this.iDevNo = iDevNo;
	}
	public void setFuncNo(int iFuncNo){
		this.iFuncNo = iFuncNo;
	}
	public void setPlugged(int iPlugged){
		this.iPlugged = iPlugged;
	}

	public int getVendorId(){
		return this.iVendorId;
	}
	public int getDeviceId(){
		return this.iDeviceId;
	}
	public int getSerialNo(){
		return this.iSerialNo;
	}
	public int getBusType(){
		return this.iBusType;
	}
	public int getBusNo(){
		return this.iBusNo;
	}
	public int getDevNo(){
		return this.iDevNo;
	}
	public int getFuncNo(){
		return this.iFuncNo;
	}
	public int getPlugged(){
		return this.iPlugged;
	}
}
