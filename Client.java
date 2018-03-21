public class Client {
    

    
    private void messageHandler(String input) {

    }

    private void GOSSIP(String s) {
        //if first message and timestamp, generateHash(message, timestamp)
        //else generateHash(message, generateTimestamp())
    }

    private String generateTimestamp() {
        
    }

    private String generateHash(String message, String timestamp) {

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

    private void PEERS?(String s) {
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
