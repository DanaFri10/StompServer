package bgu.spl.net.srv;

import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;

public class Frame {
    private String command;
    private ConcurrentMap<String, String> headers;
    private String body;

    public Frame(String frameString)
    {
        String copyFrameStr=new String(frameString); //copy message

        int endOfCommand=copyFrameStr.indexOf('\n'); // extract header
        command=copyFrameStr.substring(0,endOfCommand);
        copyFrameStr=copyFrameStr.substring(endOfCommand+1);
        String[] frameLines = copyFrameStr.split("\n");
        headers = new ConcurrentHashMap<>();
        int i = 0;
        while(!frameLines[i].equals("")){
            String[] splitToNameAndValue=frameLines[i].split(":");
            headers.put(splitToNameAndValue[0], splitToNameAndValue[1]);
            i++;
        }
        i++;
        body = "";
        while(!frameLines[i].equals("\u0000")){
            body += frameLines[i] + "\n";
            i++;
        }
    }

    public Frame(String command, ConcurrentMap<String, String> headers, String body)
    {
        this.command=command;
        this.headers=headers;
        this.body=body;
    }

    public String getCommand()
    {
        return command;
    }

    public ConcurrentMap<String, String> getHeaders()
    {
        return headers;
    }

    public String getBody()
    {
        return body;
    }

    public String toString()
    {
        String frameStr = command;
        if(headers != null) {
            for (String headerName : headers.keySet())
                frameStr += '\n' + headerName + ':' + headers.get(headerName);
        }
        if(body.isEmpty()){
            frameStr += "\n\n\u0000";
        }
        else {
            frameStr += "\n\n" + body + '\n' + "\u0000";
        }
        
        return frameStr;
    }
}
