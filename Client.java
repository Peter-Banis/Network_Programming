import java.io.*;
import java.net.*;
import java.security.MessageDigest;
import java.text.*;
import java.util.Date;
import java.util.TimeZone;
import java.util.Base64;
//import org.apache.commons.codec.digest.DigestUtils;

public class Client {
    private Socket TCPserver;
    private InputStream in;
    private OutputStream out;
    private DatagramSocket UDPserver;
    private String initialTimestamp;
    public final boolean TCP; //true will define TCP, false will define UDP


    //We distinguish the TCP and UDP constructor by having the TCP constructor require a useless int
    public Client(String host, int port, String initialTimestamp, String initialMessage, boolean TCP) throws Exception {
        this.TCP = TCP;
        if (TCP) {
            try {
                TCPserver = new Socket(host, port);
                in = TCPserver.getInputStream();
                out = TCPserver.getOutputStream();
            } catch (Exception e) {
                System.out.println(e);
                throw new Exception();
            }
        } else {
            try {
                UDPserver = new DatagramSocket();
                UDPserver.connect(InetAddress.getByName(host), port);
            } catch (Exception e) {
                System.out.println(e);
                throw new Exception();
            }
        }
        this.initialTimestamp = initialTimestamp;
        if (initialMessage != null) {
            messageHandler(initialMessage);
        }      
    }
    
    //use to test invidual functions without needing connections
    public Client() {
        TCP = false;
    }

   
    private void messageHandler(String input) {
        if (input.equals("PEERS?")) {
            PEERS(input);
        } else if (input.indexOf("PEER") == 0) {
            PEER(input);
        } else {
            GOSSIP(input);
        }
    }

    private void GOSSIP(String s) {
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
        sendMessage(toSend);
    }

    private String generateTimestamp() {
        TimeZone.setDefault(TimeZone.getTimeZone("UTC"));
        SimpleDateFormat formatter = new SimpleDateFormat("YYYY-MM-dd-HH-mm-ss-SSS");
        String timestamp = formatter.format(new Date()); timestamp +="Z";
        return timestamp;
    }

    private String generateHash(String message, String timestamp) {
        return ""; //placeholder
    }

    private void sendMessage(String s) {
        if (TCP) {
            out.print(s);
            out.flush();
        } else {
            DatagramPacket packet = new DatagramPacket(s.getBytes(), s.getBytes().length);
            UDPserver.send(packet);
        }
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
       //networking code
        byte[] temp = new byte[1024];
        if (TCP) {
            while (read(temp) != -1) {
            //read
            }
            
        } else {
            DatagramPacket t = new DatagramPacket();
            UDPserver.receive(t)
            temp = t.getData();
        }
       //call displayResponse(responseFromServer)
       displayResponse(new String(temp));
    }


    private void displayResponse(String response) {
        System.out.println(response);
    }

    public static void main(String[] args) {
        //test
        Client client = new Client();
        System.out.println(client.generateTimestamp());
        System.out.println("Timestamp done, now testing SHA variants for results");
//        System.out.println(DigestUtils.sha256Hex("2018-01-09-16-18-20-001Z:Tom eats Jerry"));

        //process args

        //initialize connection

        //while (!EOF) loop 
        //    read input
        //    messageHandler(input)
        //end loop

    }

}
