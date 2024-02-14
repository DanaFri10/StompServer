package bgu.spl.net.impl.stomp;

import bgu.spl.net.srv.Server;

public class StompServer {
    public static void main(String[] args) {
        if(args.length < 2){
            System.out.println("Missing arguments");
            return;
        }
        int port = 0;
        try {
            port = Integer.parseInt(args[0]);
        }
        catch (Exception e){
            System.out.println("First argument must be a number");
            return;
        }
        Users users=new Users();
        if(args[1].equals("tpc")){
            Server.threadPerClient(
                    port, //port
                    () -> new StompProtocol(users), //protocol factory
                    StompEncoderDecoder::new //message encoder decoder factory
            ).serve();
        }
        else if(args[1].equals("reactor")) {
            Server.reactor(5,
                    port, //port
                    () -> new StompProtocol(users), //protocol factory
                    StompEncoderDecoder::new //message encoder decoder factory
            ).serve();
        }else{
            System.out.println("Second argument must be either tpc or reactor");
        }
        
    }
}
