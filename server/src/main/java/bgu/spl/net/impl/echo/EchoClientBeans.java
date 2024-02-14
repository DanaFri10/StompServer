package bgu.spl.net.impl.echo;

import java.io.*;
import java.net.Socket;

public class EchoClientBeans {

    public static void main(String[] args) throws IOException {

        if (args.length == 0) {
            args = new String[]{"localhost", "hello"};
        }

        if (args.length < 2) {
            System.out.println("you must supply two arguments: host, message");
            System.exit(1);
        }

        //BufferedReader and BufferedWriter automatically using UTF-8 encoding
        try (Socket sock = new Socket(args[0], 8888);
                BufferedReader in = new BufferedReader(new InputStreamReader(sock.getInputStream()));
                BufferedWriter out = new BufferedWriter(new OutputStreamWriter(sock.getOutputStream()))) {
                System.out.println("sending message to server");
                String con = "CONNECT\naccept-version:1.2\nhost:stomp.cs.bgu.ac.il\nlogin:dohba\npasscode:buddy\n\n~@";
            System.out.println(con);
                out.write(con);
                out.flush();

                System.out.println("awaiting response");
                while(!in.ready()){

                }
                while(in.ready()){
                    System.out.print((char)in.read());
                }
            con = "SEND\ndestination:b\n\nHELLO BUDDY WHY WHY WHY NO\n~@";
            System.out.println("\n"+con);
            out.write(con);
            out.flush();

            System.out.println("awaiting response");
            while(!in.ready()){

            }
            while(in.ready()){
                System.out.print((char)in.read());
            }
            con = "DISCONNECT\nreceipt:17\n\n~@";
            System.out.println("\n"+con);
            out.write(con);
            out.newLine();
            out.flush();

            System.out.println("awaiting response");
            while(!in.ready()){

            }
            while(in.ready()){
                System.out.print((char)in.read());
            }
        }
    }
}
