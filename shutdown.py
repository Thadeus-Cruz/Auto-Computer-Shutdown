import http.server
import socketserver
import os
import sys
import socket                       # For reverse DNS lookup
import subprocess                   # For the new shutdown command
from datetime import datetime       # To timestamp the log
from getmac import get_mac_address  # To get the MAC address

PORT = 8000
LOG_FILE = "shutdown_log.txt"       # Define log file name

class ShutdownHandler(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/shutdown':
            
            # --- Start of new logging code ---
            try:
                ip = self.client_address[0]
                
                # Try to get hostname from IP (reverse DNS)
                try:
                    hostname, _, _ = socket.gethostbyaddr(ip)
                except socket.herror:
                    hostname = "Unknown (Reverse DNS failed)"
                
                # Try to get MAC address from IP
                mac = get_mac_address(ip=ip)
                if mac is None:
                    mac = "Unknown (Not in ARP table)"

                # Get current time
                now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                
                # Format the log message
                log_message = f"[{now}] Shutdown requested by: IP={ip}, Host={hostname}, MAC={mac}\n"
                
                # Print to console and write to file
                print(log_message)
                with open(LOG_FILE, "a") as log_file:
                    log_file.write(log_message)
                    
            except Exception as e:
                print(f"Error during logging: {e}")
            # --- End of new logging code ---
            
            # Send the original response
            self.send_response(200)
            self.send_header('Content-type', 'text/plain')
            self.end_headers()
            self.wfile.write(b'Shutting down the PC...')
            
            # Execute shutdown
            print("Shutdown request received. Shutting down...")
            
            # Use the full path and the modern 'subprocess' module
            subprocess.run([r"C:\Windows\System32\shutdown.exe", "/s", "/f", "/t", "0"])
            
        else:
            self.send_response(404)
            self.end_headers()

# --- Main execution ---
try:
    with socketserver.TCPServer(("", PORT), ShutdownHandler) as httpd:
        print(f"Shutdown server running on port {PORT}")
        print(f"Logging requests to {LOG_FILE}")
        httpd.serve_forever()
except OSError as e:
    print(f"Error: Could not bind to port {PORT}. Is it already in use?")
    print("If you just ran this, you might need to wait a minute for the port to be freed.")
except KeyboardInterrupt:
    print("\nServer stopped.")
    sys.exit(0)