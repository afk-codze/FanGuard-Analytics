//List of CustomChart
var chart_list = [];
var interval_id = null;
var anomaly_table;




$(document).ready(function () {
    $('#realtime_submit').submit(function (event) {
        event.preventDefault();
        const selectedDeviceId = $('#deviceId').val(); // Get selected device ID

        if (!selectedDeviceId) {
            console.warn('No device ID selected');
            return; // Optionally prevent the request if no ID is selected
        }


        clearInterval(interval_id);
        generateGraphs(selectedDeviceId);
        generateAnomaliesTable(selectedDeviceId);
        interval_id = setInterval(function() {
            generateGraphs(selectedDeviceId);  // Update graphs
            generateAnomaliesTable(selectedDeviceId);  // Update anomalies table
        }, 1000);
    });


    anomaly_table = setAnomalyTable();

});

function generateGraphs(selectedDeviceId) {
    $.ajax({
        url: '/api/datastream/realtime',
        type: 'GET',
        data: {
            deviceId: selectedDeviceId
        },
        success: function (data) {
            console.log(data);

            const powerData = data[0];
            const rmsData = data[1];

            let hasData = false;

            if (powerData && powerData.length !== 0) {
                handleChartCreationAndUpdate("Power", powerData, true);
                hasData = true;
            }

            if (rmsData && rmsData.length !== 0) {
                handleChartCreationAndUpdate("RMS_X", rmsData, true);
                handleChartCreationAndUpdate("RMS_Y", rmsData, true);
                handleChartCreationAndUpdate("RMS_Z", rmsData, true);
                hasData = true;
            }

            if (hasData) {
                disableAnimation();
            } else {
                destroyCharts();
            }
        },
        error: function () {
            console.log('Error fetching data');
            destroyCharts();
        }
    });
}

function generateAnomaliesTable(selectedDeviceId){
    $.ajax({
        url: '/api/anomalies/realtime',
        type: 'GET',
        data: {
            deviceId: selectedDeviceId
        },
        success: function (data) {
            // Assuming data is an array of objects
            console.log(data);
            if (data.length !== 0) {
                fillAnomaliesTable(data) // fill anomalies table with last hour data (limit?)
                disableAnimation();
            }else{

            }

        },
        error: function () {
            console.log('Error fetching data');
        }
    });
}

function disableAnimation() {
    chart_list.forEach(c => {
        c.chart.options.animation.duration = 0;
        c.chart.update();
    })
}

function setAnomalyTable() {
    return $('#anomalies_table').DataTable({
        "paging": true,
        "searching": false, // Disable search box
        "info": false, // Disable info text,
        "pageLength": 15,
        language: {
            entries: {
                _: 'anomalies',
                1: 'anomaly'
            },
            info: ""
        },
        "ordering": false,
        "lengthChange": false    // Hides the 10/25/50 dropdown
    });
}

// Reset graphs
$('#reset').click(function (event) {
    event.preventDefault();
    chart_list.forEach(c => {
        $('#'.concat(c.canva_name)).attr("class", "0");
        c.chart.destroy();
    })
    chart_list = [];
    if(anomaly_table != null){
        anomaly_table.clear().draw();
    }
    firstCallTimestamp = null;
    clearInterval(interval_id);
})
