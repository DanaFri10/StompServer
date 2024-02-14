package bgu.spl.net.impl.echo;

import bgu.spl.net.api.MessagingProtocol;
import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.ConnectionsImpl;

import java.time.LocalDateTime;

public class EchoProtocol implements StompMessagingProtocol<String> {

    private Connections<String> connections;
    private int connectionId;
    private boolean shouldTerminate = false;

    @Override
    public void process(String msg) {
        shouldTerminate = "bye".equals(msg);
        System.out.println("[" + LocalDateTime.now() + "]: " + msg);
        if(msg.contains("subscribe")){
            ((ConnectionsImpl)connections).subscribe(msg.split(" ")[1], connectionId, Integer.parseInt(msg.split(" ")[2]));
            connections.send(connectionId, "yes hello you are " + msg.split(" ")[1]);
        }
        else if(msg.contains(" ")){
            connections.send(msg.split(" ")[0], createEcho(msg.split(" ")[1]));
        }else {
            connections.send(connectionId, createEcho(msg));
        }
    }

    private String createEcho(String message) {
        String echoPart = message.substring(Math.max(message.length() - 2, 0), message.length());
        return message + " .. " + echoPart + " .. " + echoPart + " ..";
    }

    @Override
    public boolean shouldTerminate() {
        return shouldTerminate;
    }

    public void start(int connectionId, Connections<String> connections)
    {
        this.connections = connections;
        this.connectionId = connectionId;
    }
}
