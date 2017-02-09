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
            /*for(int i=0;i<255;i++) {
                for (int j = 0; j < 255; j++) {
                    String cmd = String.format("ping -n 1 -w 1 192.168.%d.%d", i,j);
                    executeCommand(cmd);
                    for(int k=0;k<255;k++) {
                        cmd = String.format("ping -n 1 -w 1 10.%d.%d.%d",i,j,k);
                        executeCommand(cmd);
                    }
                }
            }*/
            String cmd = "arp -a";
            String arp = executeCommand(cmd);
            String[] arps = arp.split("\n");
            boolean flag=false;
            for(String addr:arps){
                if(addr.contains(winmac)) {
                    System.out.println(addr);
                    flag=true;
                }
            }
            if (!flag) {
                cmd = "ipconfig";
                arp = executeCommand(cmd);
                arps = arp.split("\n");
                String mask;
                String[] ip = {"", "", "", ""};
                for (String addr : arps) {
                    if (addr.contains("IPv4 Address")) {
                        ip = addr.split(":")[1].split("\\.");
                    }
                    if (addr.contains("Subnet Mask")) {
                        mask = addr.split(":")[1];
                        String[] maskparts = mask.split("\\.");
                        if (maskparts[3].contentEquals("0")) {
                            if (maskparts[2].contentEquals("0")) {
                                if(maskparts[1].contentEquals("0")){
                                    for (int i = 1; i < 255; i++) {
                                        for (int j = 1; j < 255; j++) {
                                            for(int k=1;k<255;k++) {
                                                cmd = String.format("ping -n 1 -w 1 %s.%d.%d.%d", ip[0], i, j, k);
                                                //System.out.println(cmd);
                                                executeCommand(cmd);
                                            }
                                        }
                                    }
                                }
                                else {
                                    for (int i = 1; i < 255; i++) {
                                        for (int j = 1; j < 255; j++) {
                                            cmd = String.format("ping -n 1 -w 1 %s.%s.%d.%d", ip[0], ip[1], i, j);
                                            //System.out.println(cmd);
                                            executeCommand(cmd);
                                        }
                                    }
                                }
                            } else {
                                for (int i = 1; i < 255; i++) {
                                    cmd = String.format("ping -n 1 -w 1 %s.%s.%s.%d", ip[0], ip[1], ip[2], i);
                                    //System.out.println(cmd);
                                    executeCommand(cmd);
                                }
                            }
                        }
                    }
                }
                System.out.println("ping done");
                cmd = "arp -a";
                arp = executeCommand(cmd);
                arps = arp.split("\n");
                for (String addr : arps) {
                    if (addr.contains(winmac))
                        System.out.println(addr);
                }
            }
        }
        else{
            String unmac = mac.replaceAll(":|-",":");
            String cmd = "ping -c 1 -b 255.255.255.255";
            executeCommand(cmd);
            System.out.println("ping done");
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

