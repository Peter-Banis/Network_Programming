import java.io.*;
import java.net.*;
import java.security.MessageDigest;
import java.text.*;
import java.util.Date;
import java.util.Base64;


public class Client {
    private Socket TCPserver;
    private InputStream in;
    private OutputStream out;
    private DatagramSocket UDPserver;
    private String initialTimestamp;


    //We distinguish the TCP and UDP constructor by having the TCP constructor require a useless int
    public Client(String host, int port, String initialTimestamp, String initialMessage, int distinguisher) {
        try {
            TCPserver = new Socket(host, port);
            in = TCPserver.getInputStream();
            out = TCPserver.getOutputStream();
        } catch (Exception e) {
            System.out.println(e);
        }
        this.initialTimestamp = initialTimestamp;
        if (initialMessage != null) {
            messageHandler(initialMessage);
        }      
    }
    
    public Client(String host, int port, String initialTimestamp, String initialMessage) {
        try {
            UDPserver = new DatagramSocket();
            UDPserver.connect(InetAddress.getByName(host), port);
        } catch (Exception e) {
            System.out.println(e);
        }
        this.initialTimestamp = initialTimestamp;
        if (initialMessage != null) {
            messageHandler(initialMessage);
        }
    }

   
    private void messageHandler(String input) {
        if (input.equals("PEERS?")) {
            PEERS(input);
        } else if (input.indexOf("PEER") != -1) {
            PEER(input);
        } else {
            GOSSIP(input);
        }
    }

    private void GOSSIP(String s) {
        //if first message and timestamp, generateHash(message, timestamp)
        String hash, timestamp;
        timestamp = generateTimestamp();
        //if an initial timestamp was given, the first message and only the first message will use it
        if (initialTimestamp != null) {
            timestamp = initialTimestamp;
            //wipes condition so we only use an argument timestamp once, otherwise generate them
            initialTimestamp = null;
        }
        hash = generateHash(s, timestamp);
        //               GOSSIP:   [sha]   :     [timestamp] :  [message] %
        String toSend = "GOSSIP:" + hash + ":" + timestamp + ":" + s + "%";
        if (TCPserver != null) {

        } else { //construct datagram packet and send it 
 
        }
        
    }

    private String generateTimestamp() {
        SimpleDateFormat formatter = new SimpleDateFormat("YYYY-MM-dd-HH-mm-ss-SSS");
        String timestamp = formatter.format(new Date()); timestamp +="Z"; //This is cheating until I figure out how to properly end timezone
        return timestamp;
    }

    private String generateHash(String message, String timestamp) {
        return ""; //placeholder
    }

    private void sendMessage(String s) {

    }
    /*
     *I'd prefer the user types exactly what is sent online, except for the hash
     *Thus a PEER string is provided by the user, we do not prompt for input
     * Further, we can let the server handle error checking for us, so we don't
     * need to verify s for a valid form here
     */
    private void PEER(String s) {
        sendMessage(s);
    }

    private void PEERS(String s) {
        sendMessage(s);
        receiveResponse();   
    }
    
    private void receiveResponse() {
        
    }


    private void displayResponse() {

    }

    public static void main(String[] args) {
        //process args

        //initialize connection

        //while (!EOF) loop 
        //    read input
        //    messageHandler(input)
        //end loop

    }

}
