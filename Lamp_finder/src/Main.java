import javax.management.Query;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.*;
import java.util.Scanner;

/**
 * Created by yurra on 12.12.2016.
 */

public class Main {
    public static void main(String[] args) throws UnknownHostException, SocketException {
        String os = System.getProperty("os.name");
        System.out.println(os);
        Scanner scanner = new Scanner(System.in);
        System.out.print("enter mac:");
        String mac = scanner.nextLine();
        //System.out.print("enter subnet:");
        //String subnet = scanner.nextLine();
        scanner.close();
        if(os.contains("Windows")){
            String winmac = mac.replaceAll(":|-","-");
            //String cmd = "ping -n 1 "+subnet+".255";
            String cmd = "ping -n 1 192.255.255.255";
            executeCommand(cmd);
            cmd = "ping -n 1 10.255.255.255";
            executeCommand(cmd);
            cmd = "arp -a";
            String arp = executeCommand(cmd);
            String[] arps = arp.split("\n");
            for(String addr:arps){
                if(addr.contains(winmac))
                    System.out.println(addr);
            }
        }
        else{
            String unmac = mac.replaceAll(":|-",":");
            String cmd = "ping -c 1 -b 255.255.255.255";
            executeCommand(cmd);
            cmd = "arp";
            String arp = executeCommand(cmd);
            String[] arps = arp.split("\n");
            for(String addr:arps) {
                if (addr.contains(unmac))
                    System.out.println(addr);
            }
        }
    }
    private static String executeCommand(String command) {

        StringBuffer output = new StringBuffer();

        Process p;
        try {
            p = Runtime.getRuntime().exec(command);
            p.waitFor();
            BufferedReader reader =
                    new BufferedReader(new InputStreamReader(p.getInputStream()));

            String line = "";
            while ((line = reader.readLine())!= null) {
                output.append(line + "\n");
            }

        } catch (Exception e) {
            e.printStackTrace();
        }

        return output.toString();

    }
}
