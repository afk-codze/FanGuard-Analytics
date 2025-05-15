var color_map = new Map();
let firstCallTimestamp = null;

function CustomChart(canva_name, chart) {
    this.canva_name = canva_name;
    this.chart = chart;
}
function Dataset(chartName, sys_data) {

    this.data = sys_data

    this.fill = false;
    this.tension = 0.1;
    this.showLine = true;

    switch (chartName) {
        case "Power" :
            this.borderColor = `rgb(54, 197, 4)`;
            this.label = "Power: mW";

            break
        case "RMS_X" :
            this.borderColor = `rgb(75, 128, 179)`;
            this.label = "RMS_X: g";

            break
        case "RMS_Y" :
            this.borderColor = `rgb(147, 112, 166)`;
            this.label = "RMS_Y: g";

            break
        case "RMS_Z" :
            this.borderColor = `rgb(200, 79, 79)`;
            this.label = "RMS_Z: g";

            break
    }
}


/*Funzione per gestire creazione o aggiornamento del grafico
* in base ad un flag impostato sull'html
* se property == packets allora db_data è un array costituito da oggetti {timestamp,packet} dove packet deve essere deserializzato con JSON.parse()
* */
function handleChartCreationAndUpdate(chartName, db_data, realtime = false) {

    var chart = $('#'.concat(chartName));
    if (chart.hasClass("0")) {
        chart_list.push(createChart(db_data, createDataset(db_data, realtime,chartName), chartName));
        chart.attr("class", "1");

    } else {

        chart_list.forEach(c => {
            if (c.canva_name === chartName) {
                removeData(c.chart);
                addData(c.chart, db_data.map(r => r.timestamp), createDataset(db_data, realtime,chartName));
            }
        })

    }
}

/*Crea il grafico*/
function createChart(sys_data, dataset, title) {

    var elem = document.getElementById(title);

    var unit ;
    if(title == "Power"){
        unit = "Power : mW"
    }else{
        unit = "Vibration :  g"
    }

    var chart = new Chart(elem,
        {
            type: 'line',
            data: {
                datasets: []
            },
            options: {
                plugins: {
                    title: {
                        display: true,
                        text: title,
                        fullSize: true
                    }
                },
                scales: {
                    y: {
                        title: {
                            display: true,
                            text: unit
                        }
                    },
                    x: {
                        title: {
                            display: true,
                            text: 'Time'
                        }
                    }
                }
            }

        });
    chart.data.datasets = dataset;
    chart.update();

    return new CustomChart(title, chart);
}
/*
* Crea un array pieno dei dati necessari al grafico
* */
function createDataset(db_data, realtime, chartName) {

    var dataset = [];
    // var map = new Map();


    var axes_data = [];


    db_data.forEach(obj => {
        var y_value = null;
        switch (chartName) {
            case "Power" :
                y_value = obj.power;
                break
            case "RMS_X" :
                y_value = obj.rms_x;
                break
            case "RMS_Y" :
                y_value = obj.rms_y;
                break
            case "RMS_Z" :
                y_value = obj.rms_z;
                break
        }


        axes_data.push({x: obj.timestamp, y: y_value})
    });

    if(realtime){
        axes_data = timestampToElapsedTime(axes_data);
    }
    dataset.push(new Dataset(chartName, axes_data));

    return dataset;

}

function destroyCharts() {
    chart_list.forEach(c => {
        $('#'.concat(c.canva_name)).attr("class", "0");
        c.chart.destroy();
    })
    chart_list = [];
}

/*Aggiunge dati al grafico*/
function addData(chart, label, newData) {
    // chart.data.labels = label;
    chart.data.datasets = newData;
    chart.update();

}/*Rimuove dati dal grafico*/
function removeData(chart) {

    chart.data.labels = [];
    chart.data.datasets = [];
    chart.update();
}



/*
* Anomalies Table handling
* */

function fillAnomaliesTable(data) {


    var tableData = [];
    var anomalies = data;
    anomaly_table.clear().draw();

    anomalies.forEach(anomaly => {

        var anomalyDetails = "";
        var anomalyData = [];
        console.log(anomaly);

        anomalyDetails = `Anomaly with values:  x: ${anomaly.rms_x.toString()}  y: ${anomaly.rms_y.toString()}  z: ${anomaly.rms_z.toString()} `

        anomalyDetails = anomalyDetails + "detected at " + anomaly.timestamp;
        anomalyData.push(anomalyDetails);

        tableData.push(anomalyData);
    })
    anomaly_table.rows.add(tableData).draw();


}

function timestampToElapsedTime(data) {
    if (!firstCallTimestamp) {
        firstCallTimestamp = new Date(data[0].x);
    }

    const firstTimestamp = firstCallTimestamp;

    return data.map(({ x, y }) => {
        const current = new Date(x);
        let elapsed = Math.floor((current - firstTimestamp) / 1000); // in seconds

        const hours = Math.floor(elapsed / 3600);
        elapsed %= 3600;
        const minutes = Math.floor(elapsed / 60);
        const seconds = elapsed % 60;

        let formattedTime = '';
        if (hours > 0) formattedTime += `${hours}h `;
        if (minutes > 0 || hours > 0) formattedTime += `${minutes}m `;
        formattedTime += `${seconds}s`;

        return {
            x: formattedTime.trim(),
            y
        };
    });
}