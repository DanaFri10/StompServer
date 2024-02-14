package bgu.spl.net.impl.stomp;


import java.io.IOException;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.ConnectionsImpl;
import bgu.spl.net.srv.Frame;

public class StompProtocol implements StompMessagingProtocol<String> {

    private boolean shouldTerminate = false;
    private int connectionId;
    private Connections<String> connections;
    private Users users;
    private String userLogin;

    public StompProtocol(Users users)
    {
        this.users=users;
        userLogin = "";
    }

    public void start(int connectionId, Connections<String> connections)
    {
        this.connectionId=connectionId;
        this.connections=connections;
    }

    @Override
    public void process(String msg) {
        Frame message=new Frame(msg);
        
        Frame receipt=null;
        if(message.getHeaders().containsKey("receipt"))
        {
            ConcurrentMap<String, String> headers=new ConcurrentHashMap<String, String>();
            headers.put("receipt-id", message.getHeaders().get("receipt"));
            receipt=new Frame("RECEIPT", headers, "");
        }

        Frame answer=null;
        String command=message.getCommand();
        switch(command)
        {
            case "CONNECT" : 
            {
                processConnectFrame(message);
                break;
            }
            case "SEND" : 
            {
                processSendFrame(message); 
                break;
            }
            case "SUBSCRIBE" : 
            {
                processSubscribeFrame(message);
                break;
            }
            case "UNSUBSCRIBE" : 
            {
                processUnsubscribeFrame(message);
                break;
            }
            case "DISCONNECT" : 
            {
                processDisconnectFrame(message, receipt);
                break;
            }
            default : 
                answer = new Frame("ERROR", null, "There is no such frame. Frame sent:\n" + message);
        }

        if(answer!=null) {
            connections.send(connectionId, answer.toString());
            if (answer.getCommand().equals("ERROR")) disconnect();
        }
        if(receipt!=null && !command.equals("DISCONNECT")) 
            connections.send(connectionId, receipt.toString());
    }

    private boolean checkFrame(String[] requiredHeaders, Frame f, boolean shouldHaveBody)
    {
        boolean legalFrame = true;
        String errorMsg;
        
        // check if all required headers exist
        for(String header : requiredHeaders)
        {
            if(!f.getHeaders().containsKey(header))
            {
                errorMsg = "The message:\n-----\n" + f.toString() + "\n-----\n";
                errorMsg += "Did not contain a " + header + " header, which is REQUIRED for message propagation."; 
                sendErrorFrame(f, errorMsg);
                legalFrame = false; 
            }
        }

        // check if there is a message body when there should not be and vice versa
        if((!shouldHaveBody) && (!f.getBody().equals("")))
        {
            errorMsg = "The message:\n-----\n" + f.toString() + "\n-----\n";
            if(shouldHaveBody)
                errorMsg += "should have a message body but does not.";
            else
                errorMsg += "has a message body but should not.";
            sendErrorFrame(f, errorMsg);
            legalFrame = false;
        }
        
        if(!legalFrame) disconnect();

        return legalFrame;
    }

    private void disconnect()
    {
        try {
            users.logout(userLogin);
            connections.disconnect(connectionId);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
        shouldTerminate = true;
    }

    private void sendErrorFrame(Frame f, String errorMsg)
    {
        ConcurrentMap<String, String> errorHeaders = new ConcurrentHashMap<String, String>();
        if(f.getHeaders().containsKey("receipt"))
            errorHeaders.put("receipt-id", f.getHeaders().get("receipt"));
        errorHeaders.put("message", "malformed frame recieved");

        Frame error = new Frame("ERROR", errorHeaders, errorMsg);
        connections.send(connectionId, error.toString()); 
    }

    private boolean checkConnectFrame(Frame f)
    {
        boolean legalFrame = true;
        String[] requiredHeaders = {"accept-version", "host", "login", "passcode"};
        if(!checkFrame(requiredHeaders, f, false))
            legalFrame = false;
        
        if(f.getHeaders().containsKey("accept-version") && !f.getHeaders().get("accept-version").equals("1.2"))
        {
            String errorMsg = "The message:\n-----\n" + f.toString() + "\n-----\n";
            errorMsg += "accept-version value should be 1.2.";
            sendErrorFrame(f, errorMsg);
            legalFrame = false;
        }

        if(f.getHeaders().containsKey("host") && !f.getHeaders().get("host").equals("stomp.cs.bgu.ac.il"))
        {
            String errorMsg = "The message:\n-----\n" + f.toString() + "\n-----\n";
            errorMsg += "host value should be stomp.cs.bgu.ac.il.";
            sendErrorFrame(f, errorMsg);
            legalFrame = false;
        }

        // assuming login and password values contain only English and numeric

        if(!legalFrame) disconnect();
        return legalFrame;
    }

    private void processConnectFrame(Frame f)
    {
        if(checkConnectFrame(f))
        {
            Frame response = users.login(f.getHeaders().get("login"), f.getHeaders().get("passcode"), connectionId, f.getHeaders().get("accept-version"));
            if(!response.getCommand().equals("ERROR"))
                userLogin = f.getHeaders().get("login");
            connections.send(connectionId, response.toString());
            if(response.getCommand().equals("ERROR")){
                try {
                    connections.disconnect(connectionId);
                } catch (IOException e) {
                    throw new RuntimeException(e);
                }
                shouldTerminate = true;
            }
        }
    }

    private boolean checkSendFrame(Frame f)
    {
        boolean legalFrame = true;
        String[] requiredHeaders = {"destination"};
        if(!checkFrame(requiredHeaders, f, true))
            legalFrame = false;
        
        if(!f.getHeaders().containsKey("destination") ||  f.getHeaders().get("destination").charAt(0) != '/')
        {
            String errorMsg = "The message:\n-----\n" + f.toString() + "\n-----\n";
            errorMsg += "destination value should start with /.";
            sendErrorFrame(f, errorMsg);
            legalFrame = false;
        }

        if(!legalFrame) disconnect();
        return legalFrame;
    }

    private void processSendFrame(Frame f)
    {
        if(checkSendFrame(f))
        {
            String channel = f.getHeaders().get("destination");

            if(connections.getSubscriptionId(channel, connectionId) == -1) // client is not subscribed to the channel
            {
                String errorMsg = "Attempted to send message to channel: " + channel + " but not subscribed to it.";
                sendErrorFrame(f, errorMsg);
                disconnect();
            }
            else
            {
                for(int connectionId : connections.getSubscribed(channel)){
                    ConcurrentMap<String, String> headers=new ConcurrentHashMap<String, String>();
                    Frame messageFrame = new Frame("MESSAGE", headers, f.getBody());
                    headers.put("subscription", ""+connections.getSubscriptionId(channel, connectionId));
                    headers.put("message-id", ""+((ConnectionsImpl<String>)connections).getMessageId());
                    headers.put("destination", channel);
                    connections.send(connectionId, messageFrame.toString());
                }
            }
        }
    }

    private boolean checkSubscribeFrame(Frame f)
    {
        boolean legalFrame = true;
        String[] requiredHeaders1 = {"destination", "id"};
        if(!checkFrame(requiredHeaders1, f, false))
            legalFrame = false;
        
        if(f.getHeaders().containsKey("destination") &&  f.getHeaders().get("destination").charAt(0) != '/')
        {
            String errorMsg = "The message:\n-----\n" + f.toString() + "\n-----\n";
            errorMsg += "destination value should start with /.";
            sendErrorFrame(f, errorMsg);
            legalFrame = false;
        }

        if(f.getHeaders().containsKey("id") && !isNumber(f.getHeaders().get("id")))
        {
            String errorMsg = "The message:\n-----\n" + f.toString() + "\n-----\n";
            errorMsg += "id value should be numeric.";
            sendErrorFrame(f, errorMsg);
            legalFrame = false;
        }

        if(!legalFrame) disconnect();
        return legalFrame;
    }

    private void processSubscribeFrame(Frame f)
    {
        if(checkSubscribeFrame(f))
        {
            String dest = f.getHeaders().get("destination");
            if(connections.getSubscriptionId(dest, connectionId) != -1) // client is not subscribed to the channel
            {
                String errorMsg = "Already subscribed to channel " + dest + ".";
                sendErrorFrame(f, errorMsg);
                disconnect();
            }
            else
            {
                connections.subscribe(dest, connectionId, Integer.parseInt(f.getHeaders().get("id")));
            }
        }
    }

    private boolean checkUnsubscribeFrame(Frame f)
    {
        boolean legalFrame = true;
        String[] requiredHeaders = {"id"};
        if(!checkFrame(requiredHeaders, f, false))
            legalFrame = false;

        if(f.getHeaders().containsKey("id") && !isNumber(f.getHeaders().get("id")))
        {
            String errorMsg = "The message:\n-----\n" + f.toString() + "\n-----\n";
            errorMsg += "id value should be numeric.";
            sendErrorFrame(f, errorMsg);
            legalFrame = false;
        }

        if(!legalFrame) disconnect();
        return legalFrame;
    }

    private void processUnsubscribeFrame(Frame f)
    {
        if(checkUnsubscribeFrame(f))
        {
            int subId = Integer.parseInt(f.getHeaders().get("id"));
            if(!((ConnectionsImpl<String>)connections).isClientSubscribed(connectionId, subId)) // client does not have a subscription with the sub id
            {
                String errorMsg = "Not subscribed to any topic with the subscription id " + subId + ".";
                sendErrorFrame(f, errorMsg);
                disconnect();
            }
            else
            {
                connections.unsubscribe(subId, connectionId);
            }
        }
    }

    private boolean checkDisconnectFrame(Frame f)
    {
        boolean legalFrame = true;
        String[] requiredHeaders = {"receipt"};
        if(!checkFrame(requiredHeaders, f, false))
            legalFrame = false;

        if(f.getHeaders().containsKey("receipt") && !isNumber(f.getHeaders().get("receipt")))
        {
            String errorMsg = "The message:\n-----\n" + f.toString() + "\n-----\n";
            errorMsg += "receipt value should be numeric.";
            sendErrorFrame(f, errorMsg);
            legalFrame = false;
        }

        if(!legalFrame) disconnect();
        return legalFrame;
    }

    private void processDisconnectFrame(Frame f, Frame receipt)
    {
        if(checkDisconnectFrame(f))
        {
            if(receipt!=null)
            {
                connections.send(connectionId, receipt.toString());
            }
            users.logout(userLogin);
            try{
                connections.disconnect(connectionId);
            }
            catch(IOException e) {}
            shouldTerminate = true;
        }
    }


    @Override
    public boolean shouldTerminate() {
        return shouldTerminate;
    }

    private boolean isNumber(String s)
    {
        if(s == null) return false;
        try {
            Integer.parseInt(s);
        }
        catch(NumberFormatException e)
        {
            return false;
        }
        return true;
    }
}
