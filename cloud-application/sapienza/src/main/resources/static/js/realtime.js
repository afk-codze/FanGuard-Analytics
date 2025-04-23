//List of CustomChart
var chart_list = [];
var interval_id = null;
var anomaly_table;




$(document).ready(function () {
    $('#realtime_submit').submit(function (event) {
        event.preventDefault();


        clearInterval(interval_id);
        generateGraphs();
        generateAnomaliesTable();
        interval_id = setInterval(function() {
            generateGraphs();  // Update graphs
            generateAnomaliesTable();  // Update anomalies table
        }, 5000);
    });


    anomaly_table = setAnomalyTable();

});

function generateGraphs() {


    $.ajax({
        url: '/api/datastream/realtime',
        type: 'GET',
        data: {
        },
        success: function (data) {
            // Assuming data is an array of objects
            if (data.length !== 0) {
                handleChartCreationAndUpdate("Power", data, true);
                disableAnimation();
            }else{
                destroyCharts();
            }

        },
        error: function () {
            console.log('Error fetching data');
        }
    });
}

function generateAnomaliesTable(){
    $.ajax({
        url: '/api/anomalies/realtime',
        type: 'GET',
        data: {
        },
        success: function (data) {
            // Assuming data is an array of objects
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
