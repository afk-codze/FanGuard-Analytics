var sys_data;

//List of CustomChart
var chart_list = [];
var anomaly_table;



$(document).ready(function () {

    setDefaultDates();

    $('#log_submit').submit(function (event) {
        event.preventDefault();

        generateGraphs();
        generateAnomaliesTable();
    });

    anomaly_table = setAnomalyTable();




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
                console.log(data);
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
    console.log(end_timestamp);


    $.ajax({
        url: '/api/anomalies/timeRange',
        type: 'GET',
        data: {
            start: start_timestamp,
            end: end_timestamp
        },
        success: function (data) {
            // Assuming data is an array of objects
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

const setDefaultDates = function() {

    // Set the end date (today at midnight)
    const endDate = new Date();
    endDate.setDate(endDate.getDate() + 1); // Previous day
    endDate.setHours(0, 0, 0, 0); // Midnight today

    // Set the start date (previous day at midnight)
    const startDate = new Date();
    startDate.setDate(startDate.getDate() - 1); // Previous day
    startDate.setHours(0, 0, 0, 0); // Midnight previous day

    // Format the dates to match the input format (YYYY-MM-DDTHH:mm)
    const formatDate = (date) => {
        date.setHours(0, 0, 0, 0);

        const year = date.getFullYear();
        const month = String(date.getMonth() + 1).padStart(2, '0'); // Ensure 2 digits
        const day = String(date.getDate()).padStart(2, '0'); // Ensure 2 digits
        const hours = String(date.getHours()).padStart(2, '0'); // Ensure 2 digits
        const minutes = String(date.getMinutes()).padStart(2, '0'); // Ensure 2 digits

        // Return in the format YYYY-MM-DDTHH:mm
        return `${year}-${month}-${day}T${hours}:${minutes}`;
    };



    $('#start_time').val(formatDate(startDate));
    $('#end_time').val(formatDate(endDate));
};

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
    destroyCharts();
    if (anomaly_table != null) {
        anomaly_table.clear().draw();
    }

})




