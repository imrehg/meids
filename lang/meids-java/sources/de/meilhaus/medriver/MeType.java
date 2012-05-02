package de.meilhaus.medriver;

public class MeType{
	private int iType;
	private int iSubtype;

	public MeType(){
		iType = 0;
		iSubtype = 0;
	}

	public void setType(int iType){
		this.iType = iType;
	}
	public void setSubtype(int iSubtype){
		this.iSubtype = iSubtype;
	}

	public int getType(){
		return this.iType;
	}
	public int getSubtype(){
		return this.iSubtype;
	}
}
