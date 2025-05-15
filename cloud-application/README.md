# Cloud 

The last component of our system is the cloud, this is the final destination of the data gathered and computed by the sensors in order to make this data usable and readable by human operators through a web application. 
Thanks to this tool an operator can achieve  a global view of the servers fans health status also they can  monitor the situation in realtime and respond quickly to anomalous situations. 

## The Architecture

In this section we give an overview of the structure and the technologies used for building the application running on the cloud, the application is called "Monitoring System". 

* __Mosquitto MQTT broker__: This is the junction point between the esp32 and the cloud, it lets transfer data from the IoT device to the cloud application.
* __Java__: The programming language used to build the web application running on the cloud, this object-oriented language is largely used to build web applications thanks to the huge amount of support and libraries it offers.
* __Java Spring__: Web development framework that supports many integrated modules that speed up the process of web development by integrating many aspects like: Dabates handling, front-end templating, MVC code structure etc.
* __PostgresSQL__ : Database Management System used to store the data coming from the esp32 and provide historical view of all the data.
* __Html - javascript__: Languages used to build the presentation level look and functionalities.

## Presentation Level

In the presentation video that you can find at this link: link, you can see how the application works from the presentation level:

There are 2 pages that can be access through the navbar placed on the top, the first page that is also the index of the application is called __realtime graphs__ in this page you can select an IoT device and then click the Submit button, at this point 4 graphs will be generated for you, these graphs are realtime so they are periodically updated with the new data coming from the IoT devices. \
The graphs generated show the power levels measured in mW and the vibration on the 3 axes measured in g, each value is timestamped with the time it was received by the cloud application, in order to make it more readable and easy to understand this timestamp has been processed in order to be shown in terms hours,minutes and seconds.\
On the right side of the page there is also a table that shows  the last 10 most recent anomalies detected by the system. \
The second page called __log__ is very similar to the first one, the functionalities are the same, but here the graphs are not realtime and by using time selectors the operator can choose a time interval of interest of which to generate the graphs and the anomalies table.

## Under the hood: the back end

The back end of the application as stated in the previous paragraph uses Java Spring as development framework, so the code is structured following the MVC pattern:

  * __Model__: ORM Model connected to the db models the information coming from the IoT devices: Anomaly, Power and RMS.
  * __View__: From a api call to the proper url it returns the html and javascript code to render the web page.
  * __Controller__: Manages interaction between Model and View, there is a controller that manages API calls from javscript code to retrieve information from the DB and a controller that manages the web pages routing.

At startup the application connects to the MQTT broker and subscribes to the interested topics, when a new message written in json fromat comes from the IoT devices into the broker, the application consumes it from it, this message is then parsed into a DTO (Data Transfer Object) using a JSON serializer and then stored on the database. \
As stated before the application uses javascript at the presentation level to handle the graph generation and other graphicacl functionalities, the data needed to fill the graphs is retrieved from the backend through an ajax request that is handled by a back end controller which processes the request, interacts with the DB to retrieve the data , process it into a service class and finally returns it as response.\
Notice that before storing the data retrieved from the MQTT broker the system verifies the hmac contained in the message, if it is not valid the message is dropped, this security mechanism is used to prevent possible replay attacks.

## How to install and run

The following steps explain the set up needed to make this application work:

### Dependencies:

* Install the open-jdk-17 and postgresql on your machine:
  ```
  sudo apt install openjdk-17-jdk postgresql -Y
  ```
* Create a new user in the psql console environment with a password
  ```
  $ sudo -i -u postgres
  $ psql
  CREATE USER your_username WITH PASSWORD 'your_password';
  CREATE DATABASE your_dbname;
  GRANT ALL PRIVILEGES ON DATABASE your_dbname TO your_username;
  \q
  $ psql -U your_username -d your_dbname -h localhost

  ```
 * Clone from github this repository, application files can be found in `cloud-application`
 
### Environment Variables:

  * Navigate to `cloud-application/src/main/resources/application.properties` 
    ```
    spring.datasource.url=jdbc:postgresql://localhost:5432/your_dbname
    spring.datasource.username=your_username
    spring.datasource.password=your_password
    spring.datasource.driver-class-name=org.postgresql.Driver

    ```
    modify these variables with your database credentials and name.

### Running:

* Start mosquitto server using from bash:

  ```
  $ mosquitto
  ```
* Build and start the application:
  ```
  $ cd ./cloud-application
  $ ./mwnw spring-boot:run
  ```
* You can now access the application at url: `http://localhost:8080/`
