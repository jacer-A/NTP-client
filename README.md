# NTP-client
An ntpClient that collects datas from the given ntpServers.

the ntpClient communicates with ntpServer (Version 4) using the packet Format:


- As parameter the ntpClient should be given the number of Requests to send to each ntpServer and the addresses of the servers to test. Example:
- 
./ntpclient 5 0.north-america.pool.ntp.org 2.north-america.pool.ntp.org 0.de.pool.ntp.org
- The Client calculates the delay, offset and dispersion. The format of the output is as follows:
{host};{n};{root_disp};{disp};{delay};{offset}
