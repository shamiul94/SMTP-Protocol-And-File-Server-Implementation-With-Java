/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package offline2;

import java.io.File;
import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

/**
 *
 * @author Heisenberg
 */
public class finalServer {

    private static int id;

    public static void main(String[] args) throws IOException {
        id = 0;
        int port = 6789;
        ServerSocket ss = new ServerSocket(port);
        System.out.println("Server Socket at port " + port + " is opened successfully.");

        File log = new File("log.txt");
        try {
            log.delete();
        } catch (Exception E) {

        }

        while (true) {
            Socket s = ss.accept();
            finalHTTPWorker worker = new finalHTTPWorker(s, id);
            Thread t = new Thread(worker);
            t.start();
            id++;
        }
    }
}
