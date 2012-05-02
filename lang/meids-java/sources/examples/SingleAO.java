package examples;

import de.meilhaus.medriver.*;

public class SingleAO {
	public static void main(String[] args){
		MeDriver drv = new MeDriver();

		try{
			drv.meOpen(0);

			int nd = drv.meQueryNumberDevices();
			if(nd > 0){
				System.out.println(nd + " devices detected by driver system.");
			}
			else{
				System.out.println("No devices detected by driver system.");
				return;
			}
			System.out.println("Search for first analog output device.");

			int ns = drv.meQuerySubdeviceByType(0, 0, MeDriver.ME_TYPE_AO, MeDriver.ME_SUBTYPE_ANY);

			MeRange range = new MeRange(MeDriver.ME_UNIT_VOLT, -10.0, 10.0);
			drv.meQueryRangeByMinMax(0, ns, 0, range);

			drv.meIOSingleConfig(
					0,
					ns,
					0,
					range.getRange(),
					MeDriver.ME_REF_AO_GROUND,
					MeDriver.ME_TRIG_CHAN_DEFAULT,
					MeDriver.ME_TRIG_TYPE_SW,
					MeDriver.ME_TRIG_EDGE_ANY,
					MeDriver.ME_IO_SINGLE_CONFIG_NO_FLAGS);

			System.out.println("Physical value = " + 5.0 + ".");
			int digital = drv.meUtilityPhysicalToDigital(
					range.getMin(),
					range.getMax(),
					range.getMaxData(),
					5.0);

			MeIOSingle singleEntry = new MeIOSingle(0, ns, 0, MeDriver.ME_DIR_OUTPUT, digital, 0, MeDriver.ME_IO_SINGLE_TYPE_NO_FLAGS);
			MeIOSingle[] singleList = { singleEntry };
			drv.meIOSingle(singleList, MeDriver.ME_IO_SINGLE_NO_FLAGS);

			drv.meClose(0);
		}
		catch(java.io.IOException e){
			e.printStackTrace(System.err);
		}
	}
}
