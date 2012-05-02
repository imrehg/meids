#include <stdlib.h>
#include <jni.h>
#include "MeDriver.h"
#include <medriver/medriver.h>


JNIEXPORT void JNICALL Java_de_meilhaus_medriver_MeDriver_meOpen(JNIEnv *env, jobject obj, jint iFlags){
	int err;
	char error[ME_ERROR_MSG_MAX_COUNT];
	jclass exc;

	err = meOpen(iFlags);
	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return;
		(*env)->ThrowNew(env, exc, error);
		return;
	}

	return;
}


JNIEXPORT void JNICALL Java_de_meilhaus_medriver_MeDriver_meClose(JNIEnv *env, jobject obj, jint iFlags){
	int err;
	char error[ME_ERROR_MSG_MAX_COUNT];
	jclass exc;

	err = meClose(iFlags);
	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return;
		(*env)->ThrowNew(env, exc, error);
		return;
	}

	return;
}


JNIEXPORT void JNICALL Java_de_meilhaus_medriver_MeDriver_meLockDriver(
		JNIEnv *env,
	   	jobject obj,
	   	jint iLock,
	   	jint iFlags){
	int err;
	char error[ME_ERROR_MSG_MAX_COUNT];
	jclass exc;

	err = meLockDriver(iLock, iFlags);
	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return;
		(*env)->ThrowNew(env, exc, error);
		return;
	}

	return;
}


JNIEXPORT void JNICALL Java_de_meilhaus_medriver_MeDriver_meLockDevice(
		JNIEnv *env,
	   	jobject obj,
	   	jint iDevice,
	   	jint iLock,
	   	jint iFlags){
	int err;
	char error[ME_ERROR_MSG_MAX_COUNT];
	jclass exc;

	err = meLockDevice(iDevice, iLock, iFlags);
	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return;
		(*env)->ThrowNew(env, exc, error);
		return;
	}

	return;
}


JNIEXPORT void JNICALL Java_de_meilhaus_medriver_MeDriver_meLockSubdevice(
		JNIEnv *env,
	   	jobject obj,
	   	jint iDevice,
	   	jint iSubdevice,
	   	jint iLock,
	   	jint iFlags){
	int err;
	char error[ME_ERROR_MSG_MAX_COUNT];
	jclass exc;

	err = meLockSubdevice(iDevice, iSubdevice, iLock, iFlags);
	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return;
		(*env)->ThrowNew(env, exc, error);
		return;
	}

	return;
}


JNIEXPORT void JNICALL Java_de_meilhaus_medriver_MeDriver_meIOIrqStart(
		JNIEnv *env,
	   	jobject obj,
	   	jint iDevice,
	   	jint iSubdevice,
	   	jint iChannel,
	   	jint iIrqSource,
	   	jint iIrqEdge,
	   	jint iIrqArg,
	   	jint iFlags){
	int err;
	char error[ME_ERROR_MSG_MAX_COUNT];
	jclass exc;

	err = meIOIrqStart(iDevice, iSubdevice, iChannel, iIrqSource, iIrqEdge, iIrqArg, iFlags);
	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return;
		(*env)->ThrowNew(env, exc, error);
		return;
	}

	return;
}


JNIEXPORT void JNICALL Java_de_meilhaus_medriver_MeDriver_meIOIrqStop(
		JNIEnv *env,
	   	jobject obj,
	   	jint iDevice,
	   	jint iSubdevice,
	   	jint iChannel,
	   	jint iFlags){
	int err;
	char error[ME_ERROR_MSG_MAX_COUNT];
	jclass exc;

	err = meIOIrqStop(iDevice, iSubdevice, iChannel, iFlags);
	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return;
		(*env)->ThrowNew(env, exc, error);
		return;
	}

	return;
}


JNIEXPORT void JNICALL Java_de_meilhaus_medriver_MeDriver_meIOIrqWait(
		JNIEnv *env,
	   	jobject obj,
	   	jint iDevice,
	   	jint iSubdevice,
	   	jint iChannel,
	   	jobject irq,
	   	jint iTimeOut,
	   	jint iFlags){
	int err;
	char error[ME_ERROR_MSG_MAX_COUNT];
	int iIrqCount;
	int iValue;
	jclass exc;
	jclass cls = (*env)->GetObjectClass(env, irq);
	jmethodID mid;

	err = meIOIrqWait(iDevice, iSubdevice, iChannel, &iIrqCount, &iValue, iTimeOut, iFlags);
	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return;
		(*env)->ThrowNew(env, exc, error);
		return;
	}

	mid = (*env)->GetMethodID(env, cls, "setIrqCount", "(I)V");
	if(!mid) return;
	(*env)->CallVoidMethod(env, irq, mid, iIrqCount);

	mid = (*env)->GetMethodID(env, cls, "setValue", "(I)V");
	if(!mid) return;
	(*env)->CallVoidMethod(env, irq, mid, iValue);

	return;
}


JNIEXPORT void JNICALL Java_de_meilhaus_medriver_MeDriver_meIOResetDevice(
		JNIEnv *env,
	   	jobject obj,
	   	jint iDevice,
	   	jint iFlags){
	int err;
	char error[ME_ERROR_MSG_MAX_COUNT];
	jclass exc;

	err = meIOResetDevice(iDevice, iFlags);
	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return;
		(*env)->ThrowNew(env, exc, error);
		return;
	}

	return;
}


JNIEXPORT void JNICALL Java_de_meilhaus_medriver_MeDriver_meIOResetSubdevice(
		JNIEnv *env,
	   	jobject obj,
	   	jint iDevice,
	   	jint iSubdevice,
	   	jint iFlags){
	int err;
	char error[ME_ERROR_MSG_MAX_COUNT];
	jclass exc;

	err = meIOResetSubdevice(iDevice, iSubdevice, iFlags);
	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return;
		(*env)->ThrowNew(env, exc, error);
		return;
	}

	return;
}


JNIEXPORT void JNICALL Java_de_meilhaus_medriver_MeDriver_meIOSingle(
		JNIEnv *env,
	   	jobject obj,
	   	jobjectArray singleList,
	   	jint iFlags){
	meIOSingle_t *list;
	int iCount;
	int i;
	int err;
	char error[ME_ERROR_MSG_MAX_COUNT];
	jclass exc;
	jclass singleCls;
	jobject singleObj;
	jmethodID mid;

	iCount = (*env)->GetArrayLength(env, singleList);

	list = malloc( sizeof(meIOSingle_t) * iCount);
	if(!list){
		exc = (*env)->FindClass(env, "java/lang/OutOfMemoryError");
		if(!exc) return;
		(*env)->ThrowNew(env, exc, "Cannot get memory for single list");
		return;
	}

	for(i = 0; i < iCount; i++){
		singleObj = (*env)->GetObjectArrayElement(env, singleList, i);
		singleCls = (*env)->GetObjectClass(env, singleObj);

		mid = (*env)->GetMethodID(env, singleCls, "getDevice", "()I");
		if(!mid){
			free(list);
			return;
		}
		list[i].iDevice = (*env)->CallIntMethod(env, singleObj, mid);

		mid = (*env)->GetMethodID(env, singleCls, "getSubdevice", "()I");
		if(!mid){
			free(list);
			return;
		}
		list[i].iSubdevice = (*env)->CallIntMethod(env, singleObj, mid);

		mid = (*env)->GetMethodID(env, singleCls, "getChannel", "()I");
		if(!mid){
			free(list);
			return;
		}
		list[i].iChannel = (*env)->CallIntMethod(env, singleObj, mid);

		mid = (*env)->GetMethodID(env, singleCls, "getDir", "()I");
		if(!mid){
			free(list);
			return;
		}
		list[i].iDir = (*env)->CallIntMethod(env, singleObj, mid);

		mid = (*env)->GetMethodID(env, singleCls, "getValue", "()I");
		if(!mid){
			free(list);
			return;
		}
		list[i].iValue = (*env)->CallIntMethod(env, singleObj, mid);

		mid = (*env)->GetMethodID(env, singleCls, "getFlags", "()I");
		if(!mid){
			free(list);
			return;
		}
		list[i].iFlags = (*env)->CallIntMethod(env, singleObj, mid);
	}

	err = meIOSingle(list, iCount, iFlags);

	for(i = 0; i < iCount; i++){
		singleObj = (*env)->GetObjectArrayElement(env, singleList, i);
		singleCls = (*env)->GetObjectClass(env, singleObj);

		mid = (*env)->GetMethodID(env, singleCls, "setValue", "(I)V");
		if(!mid){
			free(list);
			return;
		}
		(*env)->CallVoidMethod(env, singleObj, mid, list[i].iValue);

		mid = (*env)->GetMethodID(env, singleCls, "setErrno", "(I)V");
		if(!mid){
			free(list);
			return;
		}
		(*env)->CallVoidMethod(env, singleObj, mid, list[i].iErrno);
	}

	free(list);

	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return;
		(*env)->ThrowNew(env, exc, error);
		return;
	}

	return;
}


JNIEXPORT void JNICALL Java_de_meilhaus_medriver_MeDriver_meIOSingleConfig(
		JNIEnv *env,
	   	jobject obj,
	   	jint iDevice,
	   	jint iSubdevice,
	   	jint iChannel,
	   	jint iSingleConfig,
	   	jint iRef,
	   	jint iTrigChan,
	   	jint iTrigType,
	   	jint iTrigEdge,
	   	jint iFlags){
	int err;
	char error[ME_ERROR_MSG_MAX_COUNT];
	jclass exc;

	err = meIOSingleConfig(
			iDevice,
		   	iSubdevice,
			iChannel,
			iSingleConfig,
			iRef,
			iTrigChan,
			iTrigType,
			iTrigEdge,
		   	iFlags);
	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return;
		(*env)->ThrowNew(env, exc, error);
		return;
	}

	return;
}


JNIEXPORT void JNICALL Java_de_meilhaus_medriver_MeDriver_meIOStreamConfig(
		JNIEnv *env,
	   	jobject obj,
	   	jint iDevice,
	   	jint iSubdevice,
	   	jobjectArray configListObj,
	   	jobject triggerObj,
		jint iFifoIrqThreshold,
	   	jint iFlags){
	meIOStreamConfig_t *configList;
	int iCount;
	meIOStreamTrigger_t trigger;
	int i;
	int err;
	char error[ME_ERROR_MSG_MAX_COUNT];
	jclass exc;
	jclass configCls;
	jclass triggerCls;
	jobject configObj;
	jobject tmpObj;
	jmethodID mid;

	iCount = (*env)->GetArrayLength(env, configListObj);

	configList = malloc( sizeof(meIOStreamConfig_t) * iCount);
	if(!configList){
		exc = (*env)->FindClass(env, "java/lang/OutOfMemoryError");
		if(!exc) return;
		(*env)->ThrowNew(env, exc, "Cannot get memory for config list");
		return;
	}

	for(i = 0; i < iCount; i++){
		configObj = (*env)->GetObjectArrayElement(env, configListObj, i);
		configCls = (*env)->GetObjectClass(env, configObj);

		mid = (*env)->GetMethodID(env, configCls, "getChannel", "()I");
		if(!mid){
			free(configList);
			return;
		}
		configList[i].iChannel = (*env)->CallIntMethod(env, configObj, mid);

		mid = (*env)->GetMethodID(env, configCls, "getStreamConfig", "()I");
		if(!mid){
			free(configList);
			return;
		}
		configList[i].iStreamConfig = (*env)->CallIntMethod(env, configObj, mid);

		mid = (*env)->GetMethodID(env, configCls, "getRef", "()I");
		if(!mid){
			free(configList);
			return;
		}
		configList[i].iRef = (*env)->CallIntMethod(env, configObj, mid);

		mid = (*env)->GetMethodID(env, configCls, "getFlags", "()I");
		if(!mid){
			free(configList);
			return;
		}
		configList[i].iFlags = (*env)->CallIntMethod(env, configObj, mid);
	}

	triggerCls = (*env)->GetObjectClass(env, triggerObj);

	mid = (*env)->GetMethodID(env, triggerCls, "getAcqStartTrigType", "()I");
	if(!mid){
		free(configList);
		return;
	}
	trigger.iAcqStartTrigType = (*env)->CallIntMethod(env, triggerObj, mid);

	mid = (*env)->GetMethodID(env, triggerCls, "getAcqStartTrigEdge", "()I");
	if(!mid){
		free(configList);
		return;
	}
	trigger.iAcqStartTrigEdge = (*env)->CallIntMethod(env, triggerObj, mid);

	mid = (*env)->GetMethodID(env, triggerCls, "getAcqStartTrigChan", "()I");
	if(!mid){
		free(configList);
		return;
	}
	trigger.iAcqStartTrigChan = (*env)->CallIntMethod(env, triggerObj, mid);

	mid = (*env)->GetMethodID(env, triggerCls, "getAcqStartTicks", "()J");
	if(!mid){
		free(configList);
		return;
	}
	trigger.iAcqStartTicksLow = (*env)->CallLongMethod(env, triggerObj, mid);
	trigger.iAcqStartTicksHigh = (*env)->CallLongMethod(env, triggerObj, mid) >> 32;

	mid = (*env)->GetMethodID(env, triggerCls, "getAcqStartArgs", "()[I");
	if(!mid){
		free(configList);
		return;
	}
	tmpObj = (*env)->CallObjectMethod(env, triggerObj, mid);
	(*env)->GetIntArrayRegion(env, tmpObj, 0, (*env)->GetArrayLength(env, tmpObj), trigger.iAcqStartArgs);

	mid = (*env)->GetMethodID(env, triggerCls, "getScanStartTrigType", "()I");
	if(!mid){
		free(configList);
		return;
	}
	trigger.iScanStartTrigType = (*env)->CallIntMethod(env, triggerObj, mid);

	mid = (*env)->GetMethodID(env, triggerCls, "getScanStartTicks", "()J");
	if(!mid){
		free(configList);
		return;
	}
	trigger.iScanStartTicksLow = (*env)->CallLongMethod(env, triggerObj, mid);
	trigger.iScanStartTicksHigh = (*env)->CallLongMethod(env, triggerObj, mid) >> 32;

	mid = (*env)->GetMethodID(env, triggerCls, "getScanStartArgs", "()[I");
	if(!mid){
		free(configList);
		return;
	}
	tmpObj = (*env)->CallObjectMethod(env, triggerObj, mid);
	(*env)->GetIntArrayRegion(env, tmpObj, 0, (*env)->GetArrayLength(env, tmpObj), trigger.iScanStartArgs);

	mid = (*env)->GetMethodID(env, triggerCls, "getConvStartTrigType", "()I");
	if(!mid){
		free(configList);
		return;
	}
	trigger.iConvStartTrigType = (*env)->CallIntMethod(env, triggerObj, mid);

	mid = (*env)->GetMethodID(env, triggerCls, "getConvStartTicks", "()J");
	if(!mid){
		free(configList);
		return;
	}
	trigger.iConvStartTicksLow = (*env)->CallLongMethod(env, triggerObj, mid);
	trigger.iConvStartTicksHigh = (*env)->CallLongMethod(env, triggerObj, mid) >> 32;

	mid = (*env)->GetMethodID(env, triggerCls, "getConvStartArgs", "()[I");
	if(!mid){
		free(configList);
		return;
	}
	tmpObj = (*env)->CallObjectMethod(env, triggerObj, mid);
	(*env)->GetIntArrayRegion(env, tmpObj, 0, (*env)->GetArrayLength(env, tmpObj), trigger.iConvStartArgs);

	mid = (*env)->GetMethodID(env, triggerCls, "getScanStopTrigType", "()I");
	if(!mid){
		free(configList);
		return;
	}
	trigger.iScanStopTrigType = (*env)->CallIntMethod(env, triggerObj, mid);

	mid = (*env)->GetMethodID(env, triggerCls, "getScanStopCount", "()I");
	if(!mid){
		free(configList);
		return;
	}
	trigger.iScanStopCount = (*env)->CallIntMethod(env, triggerObj, mid);

	mid = (*env)->GetMethodID(env, triggerCls, "getAcqStopTrigType", "()I");
	if(!mid){
		free(configList);
		return;
	}
	trigger.iAcqStopTrigType = (*env)->CallIntMethod(env, triggerObj, mid);

	mid = (*env)->GetMethodID(env, triggerCls, "getAcqStopCount", "()I");
	if(!mid){
		free(configList);
		return;
	}
	trigger.iAcqStopCount = (*env)->CallIntMethod(env, triggerObj, mid);

	mid = (*env)->GetMethodID(env, triggerCls, "getFlags", "()I");
	if(!mid){
		free(configList);
		return;
	}
	trigger.iFlags = (*env)->CallIntMethod(env, triggerObj, mid);

	err = meIOStreamConfig(iDevice, iSubdevice, configList, iCount, &trigger, iFifoIrqThreshold, iFlags);

	free(configList);

	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return;
		(*env)->ThrowNew(env, exc, error);
		return;
	}

	return;
}


JNIEXPORT jintArray JNICALL Java_de_meilhaus_medriver_MeDriver_meIOStreamRead(
		JNIEnv *env,
	   	jobject obj,
	   	jint iDevice,
	   	jint iSubdevice,
	   	jint iReadMode,
	   	jint iCount,
	   	jint iFlags){
	int err;
	int *piValues;
	char error[ME_ERROR_MSG_MAX_COUNT];
	jclass exc;

	piValues = malloc(sizeof(int) * iCount);
	if(!piValues){
		exc = (*env)->FindClass(env, "java/lang/OutOfMemoryError");
		if(!exc) return NULL;
		(*env)->ThrowNew(env, exc, "Cannot get memory for single list");
		return NULL;
	}

	err = meIOStreamRead(iDevice, iSubdevice, iReadMode, piValues, &iCount, iFlags);
	if(err){
		free(piValues);
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return NULL;
		(*env)->ThrowNew(env, exc, error);
		return NULL;
	}

    jintArray valuesArray = (*env)->NewIntArray(env, iCount);
	if(!valuesArray){
		free(piValues);
	   	return NULL;
	}
	(*env)->SetIntArrayRegion(env, valuesArray, 0, iCount, piValues);
	free(piValues);

	return valuesArray;
}


JNIEXPORT jintArray JNICALL Java_de_meilhaus_medriver_MeDriver_meIOStreamWrite(
		JNIEnv *env,
	   	jobject obj,
	   	jint iDevice,
	   	jint iSubdevice,
	   	jint iWriteMode,
	   	jintArray valuesArray,
	   	jint iFlags){
	int err;
	int *piValues;
	int iCount;
	int n;
	char error[ME_ERROR_MSG_MAX_COUNT];
	jclass exc;

	iCount = (*env)->GetArrayLength(env, valuesArray);
	n = iCount;
	piValues = (*env)->GetIntArrayElements(env, valuesArray, 0);

	err = meIOStreamWrite(iDevice, iSubdevice, iWriteMode, piValues, &n, iFlags);
	if(err){
		(*env)->ReleaseIntArrayElements(env, valuesArray, piValues, 0);
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return NULL;
		(*env)->ThrowNew(env, exc, error);
		return NULL;
	}

    jintArray retArray = (*env)->NewIntArray(env, iCount - n);
	if(!valuesArray){
		(*env)->ReleaseIntArrayElements(env, valuesArray, piValues, 0);
	   	return NULL;
	}
	(*env)->SetIntArrayRegion(env, retArray, 0, iCount - n, &piValues[n]);
	(*env)->ReleaseIntArrayElements(env, valuesArray, piValues, 0);

	return retArray;
}


JNIEXPORT void JNICALL Java_de_meilhaus_medriver_MeDriver_meIOStreamStart(
		JNIEnv *env,
	   	jobject obj,
	   	jobject startList,
	   	jint iFlags){
	meIOStreamStart_t *list;
	int iCount;
	int i;
	int err;
	char error[ME_ERROR_MSG_MAX_COUNT];
	jclass exc;
	jclass startCls;
	jobject startObj;
	jmethodID mid;

	iCount = (*env)->GetArrayLength(env, startList);

	list = malloc( sizeof(meIOStreamStart_t) * iCount);
	if(!list){
		exc = (*env)->FindClass(env, "java/lang/OutOfMemoryError");
		if(!exc) return;
		(*env)->ThrowNew(env, exc, "Cannot get memory for start list");
		return;
	}

	for(i = 0; i < iCount; i++){
		startObj = (*env)->GetObjectArrayElement(env, startList, i);
		startCls = (*env)->GetObjectClass(env, startObj);

		mid = (*env)->GetMethodID(env, startCls, "getDevice", "()I");
		if(!mid){
			free(list);
			return;
		}
		list[i].iDevice = (*env)->CallIntMethod(env, startObj, mid);

		mid = (*env)->GetMethodID(env, startCls, "getSubdevice", "()I");
		if(!mid){
			free(list);
			return;
		}
		list[i].iSubdevice = (*env)->CallIntMethod(env, startObj, mid);

		mid = (*env)->GetMethodID(env, startCls, "getStartMode", "()I");
		if(!mid){
			free(list);
			return;
		}
		list[i].iStartMode = (*env)->CallIntMethod(env, startObj, mid);

		mid = (*env)->GetMethodID(env, startCls, "getFlags", "()I");
		if(!mid){
			free(list);
			return;
		}
		list[i].iFlags = (*env)->CallIntMethod(env, startObj, mid);
	}

	err = meIOStreamStart(list, iCount, iFlags);

	for(i = 0; i < iCount; i++){
		startObj = (*env)->GetObjectArrayElement(env, startList, i);
		startCls = (*env)->GetObjectClass(env, startObj);

		mid = (*env)->GetMethodID(env, startCls, "setErrno", "(I)V");
		if(!mid){
			free(list);
			return;
		}
		(*env)->CallVoidMethod(env, startObj, mid, list[i].iErrno);
	}

	free(list);

	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return;
		(*env)->ThrowNew(env, exc, error);
		return;
	}

	return;
}


JNIEXPORT void JNICALL Java_de_meilhaus_medriver_MeDriver_meIOStreamStop(
		JNIEnv *env,
	   	jobject obj,
	   	jobject stopList,
	   	jint iFlags){
	meIOStreamStop_t *list;
	int iCount;
	int i;
	int err;
	char error[ME_ERROR_MSG_MAX_COUNT];
	jclass exc;
	jclass startCls;
	jobject startObj;
	jmethodID mid;

	iCount = (*env)->GetArrayLength(env, stopList);

	list = malloc( sizeof(meIOStreamStop_t) * iCount);
	if(!list){
		exc = (*env)->FindClass(env, "java/lang/OutOfMemoryError");
		if(!exc) return;
		(*env)->ThrowNew(env, exc, "Cannot get memory for start list");
		return;
	}

	for(i = 0; i < iCount; i++){
		startObj = (*env)->GetObjectArrayElement(env, stopList, i);
		startCls = (*env)->GetObjectClass(env, startObj);

		mid = (*env)->GetMethodID(env, startCls, "getDevice", "()I");
		if(!mid){
			free(list);
			return;
		}
		list[i].iDevice = (*env)->CallIntMethod(env, startObj, mid);

		mid = (*env)->GetMethodID(env, startCls, "getSubdevice", "()I");
		if(!mid){
			free(list);
			return;
		}
		list[i].iSubdevice = (*env)->CallIntMethod(env, startObj, mid);

		mid = (*env)->GetMethodID(env, startCls, "getStopMode", "()I");
		if(!mid){
			free(list);
			return;
		}
		list[i].iStopMode = (*env)->CallIntMethod(env, startObj, mid);

		mid = (*env)->GetMethodID(env, startCls, "getFlags", "()I");
		if(!mid){
			free(list);
			return;
		}
		list[i].iFlags = (*env)->CallIntMethod(env, startObj, mid);
	}

	err = meIOStreamStop(list, iCount, iFlags);

	for(i = 0; i < iCount; i++){
		startObj = (*env)->GetObjectArrayElement(env, stopList, i);
		startCls = (*env)->GetObjectClass(env, startObj);

		mid = (*env)->GetMethodID(env, startCls, "setErrno", "(I)V");
		if(!mid){
			free(list);
			return;
		}
		(*env)->CallVoidMethod(env, startObj, mid, list[i].iErrno);
	}

	free(list);

	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return;
		(*env)->ThrowNew(env, exc, error);
		return;
	}

	return;
}


JNIEXPORT void JNICALL Java_de_meilhaus_medriver_MeDriver_meIOStreamStatus(
		JNIEnv *env,
	   	jobject obj,
	   	jint iDevice,
	   	jint iSubdevice,
		jint iWait,
	   	jobject status,
	   	jint iFlags){
	int err;
	int iStatus;
	int iCount;
	char error[ME_ERROR_MSG_MAX_COUNT];
	jclass exc;
	jclass cls = (*env)->GetObjectClass(env, status);
	jmethodID mid;

	err = meIOStreamStatus(iDevice, iSubdevice, iWait, &iStatus, &iCount, iFlags);
	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return;
		(*env)->ThrowNew(env, exc, error);
		return;
	}

	mid = (*env)->GetMethodID(env, cls, "setStatus", "(I)V");
	if(!mid) return;
	(*env)->CallVoidMethod(env, status, mid, iStatus);

	mid = (*env)->GetMethodID(env, cls, "setCount", "(I)V");
	if(!mid) return;
	(*env)->CallVoidMethod(env, status, mid, iCount);

	return;
}


JNIEXPORT void JNICALL Java_de_meilhaus_medriver_MeDriver_meIOStreamFrequencyToTicks(
		JNIEnv *env,
	   	jobject obj,
	   	jint iDevice,
	   	jint iSubdevice,
		jint iTimer,
	   	jobject ticks,
	   	jint iFlags){
	int err;
	double dFrequency;
	int iTicksLow;
	int iTicksHigh;
	jlong iTicks;
	char error[ME_ERROR_MSG_MAX_COUNT];
	jclass exc;
	jclass cls = (*env)->GetObjectClass(env, ticks);
	jmethodID mid;

	mid = (*env)->GetMethodID(env, cls, "getFrequency", "()D");
	if(!mid) return;
	dFrequency = (*env)->CallDoubleMethod(env, ticks, mid);

	err = meIOStreamFrequencyToTicks(iDevice, iSubdevice, iTimer, &dFrequency, &iTicksLow, &iTicksHigh, iFlags);
	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return;
		(*env)->ThrowNew(env, exc, error);
		return;
	}

	mid = (*env)->GetMethodID(env, cls, "setFrequency", "(D)V");
	if(!mid) return;
	(*env)->CallVoidMethod(env, ticks, mid, dFrequency);

	iTicks = iTicksLow + ((jlong) iTicksHigh << 32);
	mid = (*env)->GetMethodID(env, cls, "setTicks", "(J)V");
	if(!mid) return;
	(*env)->CallVoidMethod(env, ticks, mid, iTicks);

	return;
}


JNIEXPORT void JNICALL Java_de_meilhaus_medriver_MeDriver_meIOStreamTimeToTicks(
		JNIEnv *env,
	   	jobject obj,
	   	jint iDevice,
	   	jint iSubdevice,
		jint iTimer,
	   	jobject ticks,
	   	jint iFlags){
	int err;
	double dTime;
	int iTicksLow;
	int iTicksHigh;
	jlong iTicks;
	char error[ME_ERROR_MSG_MAX_COUNT];
	jclass exc;
	jclass cls = (*env)->GetObjectClass(env, ticks);
	jmethodID mid;

	mid = (*env)->GetMethodID(env, cls, "getTime", "()D");
	if(!mid) return;
	dTime = (*env)->CallDoubleMethod(env, ticks, mid);

	err = meIOStreamTimeToTicks(iDevice, iSubdevice, iTimer, &dTime, &iTicksLow, &iTicksHigh, iFlags);
	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return;
		(*env)->ThrowNew(env, exc, error);
		return;
	}

	mid = (*env)->GetMethodID(env, cls, "setTime", "(D)V");
	if(!mid) return;
	(*env)->CallVoidMethod(env, ticks, mid, dTime);

	iTicks = iTicksLow + ((jlong) iTicksHigh << 32);
	mid = (*env)->GetMethodID(env, cls, "setTicks", "(J)V");
	if(!mid) return;
	(*env)->CallVoidMethod(env, ticks, mid, iTicks);

	return;
}


JNIEXPORT jstring JNICALL Java_de_meilhaus_medriver_MeDriver_meQueryDescriptionDevice(JNIEnv *env, jobject obj, jint iDevice){
	int err;
	char error[ME_ERROR_MSG_MAX_COUNT];
	char desc[ME_DEVICE_DESCRIPTION_MAX_COUNT];
	jclass exc;

	err = meQueryDescriptionDevice(iDevice, desc, sizeof(desc));
	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc)
			return (*env)->NewStringUTF(env, "");
		(*env)->ThrowNew(env, exc, error);
		return (*env)->NewStringUTF(env, "");
	}

	return (*env)->NewStringUTF(env, desc);
}


JNIEXPORT void JNICALL Java_de_meilhaus_medriver_MeDriver_meQueryInfoDevice(JNIEnv *env, jobject obj, jint iDevice, jobject info){
	int err;
	int iVendorId;
	int iDeviceId;
	int iSerialNo;
	int iBusType;
	int iBusNo;
	int iDevNo;
	int iFuncNo;
	int iPlugged;
	char error[ME_ERROR_MSG_MAX_COUNT];
	jclass exc;
	jclass cls = (*env)->GetObjectClass(env, info);
	jmethodID mid;

	err = meQueryInfoDevice(iDevice, &iVendorId, &iDeviceId, &iSerialNo, &iBusType, &iBusNo, &iDevNo, &iFuncNo, &iPlugged);
	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return;
		(*env)->ThrowNew(env, exc, error);
		return;
	}

	mid = (*env)->GetMethodID(env, cls, "setVendorId", "(I)V");
	if(!mid) return;
	(*env)->CallVoidMethod(env, info, mid, iVendorId);

	mid = (*env)->GetMethodID(env, cls, "setDeviceId", "(I)V");
	if(!mid) return;
	(*env)->CallVoidMethod(env, info, mid, iDeviceId);

	mid = (*env)->GetMethodID(env, cls, "setSerialNo", "(I)V");
	if(!mid) return;
	(*env)->CallVoidMethod(env, info, mid, iSerialNo);

	mid = (*env)->GetMethodID(env, cls, "setBusType", "(I)V");
	if(!mid) return;
	(*env)->CallVoidMethod(env, info, mid, iBusType);

	mid = (*env)->GetMethodID(env, cls, "setBusNo", "(I)V");
	if(!mid) return;
	(*env)->CallVoidMethod(env, info, mid, iBusNo);

	mid = (*env)->GetMethodID(env, cls, "setDevNo", "(I)V");
	if(!mid) return;
	(*env)->CallVoidMethod(env, info, mid, iDevNo);

	mid = (*env)->GetMethodID(env, cls, "setFuncNo", "(I)V");
	if(!mid) return;
	(*env)->CallVoidMethod(env, info, mid, iFuncNo);

	mid = (*env)->GetMethodID(env, cls, "setPlugged", "(I)V");
	if(!mid) return;
	(*env)->CallVoidMethod(env, info, mid, iPlugged);

	return;
}


JNIEXPORT jstring JNICALL Java_de_meilhaus_medriver_MeDriver_meQueryNameDevice(JNIEnv *env, jobject obj, jint iDevice){
	int err;
	char error[ME_ERROR_MSG_MAX_COUNT];
	char name[ME_DEVICE_DESCRIPTION_MAX_COUNT];
	jclass exc;

	err = meQueryNameDevice(iDevice, name, sizeof(name));
	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc)
			return (*env)->NewStringUTF(env, "");
		(*env)->ThrowNew(env, exc, error);
		return (*env)->NewStringUTF(env, "");
	}

	return (*env)->NewStringUTF(env, name);
}


JNIEXPORT jstring JNICALL Java_de_meilhaus_medriver_MeDriver_meQueryNameDeviceDriver(JNIEnv *env, jobject obj, jint iDevice){
	int err;
	char error[ME_ERROR_MSG_MAX_COUNT];
	char name[ME_DEVICE_DESCRIPTION_MAX_COUNT];
	jclass exc;

	err = meQueryNameDeviceDriver(iDevice, name, sizeof(name));
	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc)
			return (*env)->NewStringUTF(env, "");
		(*env)->ThrowNew(env, exc, error);
		return (*env)->NewStringUTF(env, "");
	}

	return (*env)->NewStringUTF(env, name);
}


JNIEXPORT jint JNICALL Java_de_meilhaus_medriver_MeDriver_meQueryNumberDevices(JNIEnv *env, jobject obj){
	int err;
	int iNumber;
	char error[ME_ERROR_MSG_MAX_COUNT];
	jclass exc;

	err = meQueryNumberDevices(&iNumber);
	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return 0;
		(*env)->ThrowNew(env, exc, error);
		return 0;
	}

	return iNumber;
}


JNIEXPORT jint JNICALL Java_de_meilhaus_medriver_MeDriver_meQueryNumberSubdevices(JNIEnv *env, jobject obj, jint iDevice){
	int err;
	int iNumber;
	char error[ME_ERROR_MSG_MAX_COUNT];
	jclass exc;

	err = meQueryNumberSubdevices(iDevice, &iNumber);
	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return 0;
		(*env)->ThrowNew(env, exc, error);
		return 0;
	}

	return iNumber;
}


JNIEXPORT jint JNICALL Java_de_meilhaus_medriver_MeDriver_meQueryNumberChannels(JNIEnv *env, jobject obj, jint iDevice, jint iSubdevice){
	int err;
	int iNumber;
	char error[ME_ERROR_MSG_MAX_COUNT];
	jclass exc;

	err = meQueryNumberChannels(iDevice, iSubdevice, &iNumber);
	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return 0;
		(*env)->ThrowNew(env, exc, error);
		return 0;
	}

	return iNumber;
}


JNIEXPORT jint JNICALL Java_de_meilhaus_medriver_MeDriver_meQueryNumberRanges(JNIEnv *env, jobject obj, jint iDevice, jint iSubdevice, jint iUnit){
	int err;
	int iNumber;
	char error[ME_ERROR_MSG_MAX_COUNT];
	jclass exc;

	err = meQueryNumberRanges(iDevice, iSubdevice, iUnit, &iNumber);
	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return 0;
		(*env)->ThrowNew(env, exc, error);
		return 0;
	}

	return iNumber;
}


JNIEXPORT void JNICALL Java_de_meilhaus_medriver_MeDriver_meQueryRangeByMinMax(
		JNIEnv *env,
		jobject obj,
		jint iDevice,
		jint iSubdevice,
		jobject range){
	int err;
	int iUnit;
	double dMin;
	double dMax;
	int iMaxData;
	int iRange;
	char error[ME_ERROR_MSG_MAX_COUNT];
	jclass exc;
	jclass cls = (*env)->GetObjectClass(env, range);
	jmethodID mid;

	mid = (*env)->GetMethodID(env, cls, "getUnit", "()I");
	if(!mid) return;
	iUnit = (*env)->CallIntMethod(env, range, mid);

	mid = (*env)->GetMethodID(env, cls, "getMin", "()D");
	if(!mid) return;
	dMin = (*env)->CallDoubleMethod(env, range, mid);

	mid = (*env)->GetMethodID(env, cls, "getMax", "()D");
	if(!mid) return;
	dMax = (*env)->CallDoubleMethod(env, range, mid);

	err = meQueryRangeByMinMax(iDevice, iSubdevice, iUnit, &dMin, &dMax, &iMaxData, &iRange);
	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return;
		(*env)->ThrowNew(env, exc, error);
		return;
	}

	mid = (*env)->GetMethodID(env, cls, "setMin", "(D)V");
	if(!mid) return;
	(*env)->CallVoidMethod(env, range, mid, dMin);

	mid = (*env)->GetMethodID(env, cls, "setMax", "(D)V");
	if(!mid) return;
	(*env)->CallVoidMethod(env, range, mid, dMax);

	mid = (*env)->GetMethodID(env, cls, "setMaxData", "(I)V");
	if(!mid) return;
	(*env)->CallVoidMethod(env, range, mid, iMaxData);

	mid = (*env)->GetMethodID(env, cls, "setRange", "(I)V");
	if(!mid) return;
	(*env)->CallVoidMethod(env, range, mid, iRange);

	return;
}


JNIEXPORT void JNICALL Java_de_meilhaus_medriver_MeDriver_meQueryRangeInfo(
		JNIEnv *env,
	   	jobject obj,
	   	jint iDevice,
	   	jint iSubdevice,
	   	jobject range){
	int err;
	int iUnit;
	double dMin;
	double dMax;
	int iMaxData;
	int iRange;
	char error[ME_ERROR_MSG_MAX_COUNT];
	jclass exc;
	jclass cls = (*env)->GetObjectClass(env, range);
	jmethodID mid;

	mid = (*env)->GetMethodID(env, cls, "getRange", "()I");
	if(!mid) return;
	iRange = (*env)->CallIntMethod(env, range, mid);

	err = meQueryRangeInfo(iDevice, iSubdevice, iRange, &iUnit, &dMin, &dMax, &iMaxData);
	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return;
		(*env)->ThrowNew(env, exc, error);
		return;
	}

	mid = (*env)->GetMethodID(env, cls, "setUnit", "(I)V");
	if(!mid) return;
	(*env)->CallVoidMethod(env, range, mid, iUnit);

	mid = (*env)->GetMethodID(env, cls, "setMin", "(D)V");
	if(!mid) return;
	(*env)->CallVoidMethod(env, range, mid, dMin);

	mid = (*env)->GetMethodID(env, cls, "setMax", "(D)V");
	if(!mid) return;
	(*env)->CallVoidMethod(env, range, mid, dMax);

	mid = (*env)->GetMethodID(env, cls, "setMaxData", "(I)V");
	if(!mid) return;
	(*env)->CallVoidMethod(env, range, mid, iMaxData);

	return;
}



JNIEXPORT jint JNICALL Java_de_meilhaus_medriver_MeDriver_meQuerySubdeviceByType(JNIEnv *env, jobject obj, jint iDevice, jint iStartSubdevice, jint iType, jint iSubtype){
	int err;
	int iNumber;
	char error[ME_ERROR_MSG_MAX_COUNT];
	jclass exc;

	err = meQuerySubdeviceByType(iDevice, iStartSubdevice, iType, iSubtype, &iNumber);
	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return 0;
		(*env)->ThrowNew(env, exc, error);
		return 0;
	}

	return iNumber;
}


JNIEXPORT void JNICALL Java_de_meilhaus_medriver_MeDriver_meQuerySubdeviceType(
		JNIEnv *env,
	   	jobject obj,
	   	jint iDevice,
	   	jint iSubdevice,
	   	jobject type){
	int err;
	int iType;
	int iSubtype;
	char error[ME_ERROR_MSG_MAX_COUNT];
	jclass exc;
	jclass cls = (*env)->GetObjectClass(env, type);
	jmethodID mid;

	err = meQuerySubdeviceType(iDevice, iSubdevice, &iType, &iSubtype);
	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return;
		(*env)->ThrowNew(env, exc, error);
		return;
	}

	mid = (*env)->GetMethodID(env, cls, "setType", "(I)V");
	if(!mid) return;
	(*env)->CallVoidMethod(env, type, mid, iType);

	mid = (*env)->GetMethodID(env, cls, "setSubtype", "(I)V");
	if(!mid) return;
	(*env)->CallVoidMethod(env, type, mid, iSubtype);

	return;
}



JNIEXPORT jint JNICALL Java_de_meilhaus_medriver_MeDriver_meQueryVersionLibrary(JNIEnv *env, jobject obj){
	int err;
	int iNumber;
	char error[ME_ERROR_MSG_MAX_COUNT];
	jclass exc;

	err = meQueryVersionLibrary(&iNumber);
	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return 0;
		(*env)->ThrowNew(env, exc, error);
		return 0;
	}

	return iNumber;
}


JNIEXPORT jint JNICALL Java_de_meilhaus_medriver_MeDriver_meQueryVersionMainDriver(JNIEnv *env, jobject obj){
	int err;
	int iNumber;
	char error[ME_ERROR_MSG_MAX_COUNT];
	jclass exc;

	err = meQueryVersionMainDriver(&iNumber);
	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return 0;
		(*env)->ThrowNew(env, exc, error);
		return 0;
	}

	return iNumber;
}


JNIEXPORT jint JNICALL Java_de_meilhaus_medriver_MeDriver_meQueryVersionDeviceDriver(JNIEnv *env, jobject obj, jint iDevice){
	int err;
	int iNumber;
	char error[ME_ERROR_MSG_MAX_COUNT];
	jclass exc;

	err = meQueryVersionDeviceDriver(iDevice, &iNumber);
	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return 0;
		(*env)->ThrowNew(env, exc, error);
		return 0;
	}

	return iNumber;
}


JNIEXPORT jintArray JNICALL Java_de_meilhaus_medriver_MeDriver_meUtilityExtractValues(
		JNIEnv *env,
	   	jobject obj,
	   	jint iChannel,
	   	jintArray aiArray,
	   	jobjectArray configList){
	int err;
	int *piAIBuffer;
	int iAIBufferCount;
	meIOStreamConfig_t *pConfigList;
	int iConfigListCount;
	int *piChanBuffer;
	int iChanBufferCount;
	char error[ME_ERROR_MSG_MAX_COUNT];
	jclass exc;
	jclass configCls;
	jobject configObj;
	jmethodID mid;
	int i;

	iAIBufferCount = (*env)->GetArrayLength(env, aiArray);
	piAIBuffer = (*env)->GetIntArrayElements(env, aiArray, 0);

	piChanBuffer = malloc(sizeof(int) * iAIBufferCount);
	if(!piChanBuffer){
		(*env)->ReleaseIntArrayElements(env, aiArray, piAIBuffer, 0);
		exc = (*env)->FindClass(env, "java/lang/OutOfMemoryError");
		if(!exc) return 0;
		(*env)->ThrowNew(env, exc, "Cannot get memory for single list");
		return 0;
	}
	iChanBufferCount = iAIBufferCount;

	iConfigListCount = (*env)->GetArrayLength(env, configList);

	pConfigList = malloc(sizeof(meIOStreamConfig_t) * iConfigListCount);
	if(!pConfigList){
		(*env)->ReleaseIntArrayElements(env, aiArray, piAIBuffer, 0);
		free(piChanBuffer);
		exc = (*env)->FindClass(env, "java/lang/OutOfMemoryError");
		if(!exc) return 0;
		(*env)->ThrowNew(env, exc, "Cannot get memory for config list");
		return 0;
	}

	for(i = 0; i < iConfigListCount; i++){
		configObj = (*env)->GetObjectArrayElement(env, configList, i);
		configCls = (*env)->GetObjectClass(env, configObj);

		mid = (*env)->GetMethodID(env, configCls, "getChannel", "()I");
		if(!mid){
			(*env)->ReleaseIntArrayElements(env, aiArray, piAIBuffer, 0);
			free(piChanBuffer);
			free(pConfigList);
			return 0;
		}
		pConfigList[i].iChannel = (*env)->CallIntMethod(env, configObj, mid);
	}

	err = meUtilityExtractValues(
			iChannel,
		   	piAIBuffer,
		   	iAIBufferCount,
		   	pConfigList,
		   	iConfigListCount,
		   	piChanBuffer,
		   	&iChanBufferCount);
	if(err){
		(*env)->ReleaseIntArrayElements(env, aiArray, piAIBuffer, 0);
		free(piChanBuffer);
		free(pConfigList);
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return 0;
		(*env)->ThrowNew(env, exc, error);
		return 0;
	}

	(*env)->ReleaseIntArrayElements(env, aiArray, piAIBuffer, 0);
	free(piChanBuffer);
	free(pConfigList);

	return 0;
}


JNIEXPORT jint JNICALL Java_de_meilhaus_medriver_MeDriver_meUtilityPhysicalToDigital(
		JNIEnv *env,
	   	jobject obj,
	   	jdouble dMin,
	   	jdouble dMax,
	   	jint iMaxData,
	   	jdouble dPhysical){
	int err;
	int iData;
	char error[ME_ERROR_MSG_MAX_COUNT];
	jclass exc;

	err = meUtilityPhysicalToDigital(dMin, dMax, iMaxData, dPhysical, &iData);
	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return 0;
		(*env)->ThrowNew(env, exc, error);
		return 0;
	}

	return iData;
}


JNIEXPORT jdouble JNICALL Java_de_meilhaus_medriver_MeDriver_meUtilityDigitalToPhysical(
		JNIEnv *env,
	   	jobject obj,
	   	jdouble dMin,
	   	jdouble dMax,
	   	jint iMaxData,
	   	jint iData,
		jint iModuleType,
		jdouble dRefValue){
	int err;
	double dPhysical;
	char error[ME_ERROR_MSG_MAX_COUNT];
	jclass exc;

	err = meUtilityDigitalToPhysical(dMin, dMax, iMaxData, iData, iModuleType, dRefValue, &dPhysical);
	if(err){
		meErrorGetMessage(err, error, sizeof(error));
		exc = (*env)->FindClass(env, "java/io/IOException");
		if(!exc) return 0;
		(*env)->ThrowNew(env, exc, error);
		return 0;
	}

	return dPhysical;
}
