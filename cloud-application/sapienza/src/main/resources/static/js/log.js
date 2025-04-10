var sys_data;

//List of CustomChart
var chart_list = [];
var anomaly_table;


$(document).ready(function () {

    $('#log_submit').submit(function (event) {
        event.preventDefault();

        generateGraphs();
        generateAnomaliesTable();
    });

    anomaly_table = $('#anomalies_table').DataTable({
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


});

//Generate the graph
function generateGraphs() {
    var start_timestamp = $('#start_time').val();
    var end_timestamp = $('#end_time').val();


    $.ajax({
        url: '/api/datastream/timeRange',
        type: 'GET',
        data: {
            start: start_timestamp,
            end: end_timestamp
        },
        success: function (data) {
            // Assuming data is an array of objects
            if (data.length !== 0) {
                handleChartCreationAndUpdate("Temperature", data, false);
            } else {
                destroyCharts();
            }

        },
        error: function () {
            alert('Error fetching data');
        }
    });
}

//Generate the table
function generateAnomaliesTable() {

    var start_timestamp = $('#start_time').val();
    var end_timestamp = $('#end_time').val();


    $.ajax({
        url: '/api/anomalies/timeRange',
        type: 'GET',
        data: {
            start: start_timestamp,
            end: end_timestamp
        },
        success: function (data) {
            // Assuming data is an array of objects
            console.log(data);
            if (data.length !== 0) {
                fillAnomaliesTable(data) // fill anomalies table with last hour data (limit?)
                disableAnimation();
            } else {

            }

        },
        error: function () {
            alert('Error fetching data');
        }
    });
}

// Reset graphs
$('#reset').click(function (event) {
    event.preventDefault();
    destroyCharts();
    if (anomaly_table != null) {
        anomaly_table.clear().draw();
        anomaly_table = null;
    }

})




