package de.meilhaus.medriver;

public class MeIOStreamStart{
	private int iDevice;
	private int iSubdevice;
	private int iStartMode;
	private int iTimeOut;
	private int iFlags;
	private int iErrno;

	public MeIOStreamStart(){
		this.iDevice = 0;
		this.iSubdevice = 0;
		this.iStartMode = 0;
		this.iTimeOut = 0;
		this.iFlags = 0;
		this.iErrno = 0;
	}

	public MeIOStreamStart(
			int iDevice,
			int iSubdevice,
			int iStartMode,
			int iTimeOut,
			int iFlags){
		this.iDevice = iDevice;
		this.iSubdevice = iSubdevice;
		this.iStartMode = iStartMode;
		this.iFlags = iFlags;
		this.iTimeOut = iTimeOut;
		this.iErrno = 0;
	}

	public int getDevice(){
		return this.iDevice;
	}
	public int getSubdevice(){
		return this.iSubdevice;
	}
	public int getStartMode(){
		return this.iStartMode;
	}
	public int getTimeOut(){
		return this.iTimeOut;
	}
	public int getFlags(){
		return this.iFlags;
	}
	public int getErrno(){
		return this.iErrno;
	}

	public void setDevice(int iDevice){
		this.iDevice = iDevice;
	}
	public void setSubdevice(int iSubdevice){
		this.iSubdevice = iSubdevice;
	}
	public void setStartMode(int iStartMode){
		this.iStartMode = iStartMode;
	}
	public void setTimeOut(int iTimeOut){
		this.iTimeOut = iTimeOut;
	}
	public void setFlags(int iFlags){
		this.iFlags = iFlags;
	}
	public void setErrno(int iErrno){
		this.iErrno = iErrno;
	}
}
