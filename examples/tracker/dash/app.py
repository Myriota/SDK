
from datetime import datetime #, timedelta
from hashlib import new

import boto3
import dash
import dash_core_components as dcc
import dash_html_components as html
import dash_leaflet as dl
from dash.dependencies import ALL, MATCH, Input, Output, State

def scan_modules_database(database=None):
    if not database:
        database = boto3.resource('dynamodb')
    table = database.Table('MessageUnpacker-Conner-test-MessageTable-II3PEZS18R5P')
    response = table.scan(Select='ALL_ATTRIBUTES')
    database = response['Items']
    return database

def get_mostrecent_data(): #Returns a dictionary of all the highest sequence number modules e.g {'auhgig7t1hih': ('ModuleId': 'auhgig7t1hih', 'SequenceNumber': 11, 'Latitude': 30.88271), 'hgohia6hgo12': ('ModuleId': 'hgohia6hgo12', 'SequenceNumber': 9, 'Latitude': 50.25371} 
    data = {}
    corrected_data = {}
    for module in database:
        if module['ModuleId'] not in data: #If we have not yet got a highest value for this module, make this our highest
            data[module['ModuleId']] = module

        elif data[module['ModuleId']]['SequenceNumber'] < module['SequenceNumber']: #If we have found this module before but this sequence number is higher, update our data
            data[module['ModuleId']] = module

    for mod_id in data:
        corrected_data[mod_id + splitter + str(data[mod_id]['SequenceNumber'])] = data[mod_id]

    return corrected_data

def get_data(keyword): #Returns a dictionary with the current module-sequence number as keys or just the module number if there is only one on the map (Ensures keys are unique).
    global recent_markers
    filtered_markers = {}
    if keyword == None or keyword == '': #If no search item is entered, show the most recent entry from each module
        if recent_markers == {}:
            recent_markers = get_mostrecent_data()
        return recent_markers

    else:
        for marker in database:
            if keyword == marker['ModuleId']:
                filtered_markers[marker['ModuleId'] + splitter + str(marker['SequenceNumber'])] = marker

    return filtered_markers

def get_markers(ordered_modules, module_dict): #Updates current_markers to be a list of markers, and returns a marker cluster group
    global current_markers

    firstkey = None
    lastkey = None
    for module in ordered_modules:
        if firstkey == None:
            firstkey = module
        lastkey = module

    if firstkey.split(splitter)[0] == lastkey.split(splitter)[0]:
        original_marker = False
    else:
        original_marker = True
        
    marker = []
    circ_colour = None
    

    if original_marker == True: 
        for key in module_dict:
            marker.append(
                dl.Marker(
                    id = {'type': 'original_marker', 'index': key},
                    position = (module_dict[key]['Latitude'], module_dict[key]['Longitude']),
                    children=[
                        dl.Tooltip(module_dict[key]['ModuleId']),
                    ],
                )
            )

    else:
        for key in ordered_modules:
            if key == firstkey:
                circ_colour = 'Red'
            elif key == lastkey:
                circ_colour = '#85C285'
            else:
                circ_colour = 'Yellow'
            marker.append(
                dl.CircleMarker(id = {'type': 'circle_marker', 'index': key}, center=(module_dict[key]['Latitude'], module_dict[key]['Longitude']), radius=8, weight=1, color='Black', fill=True, fillOpacity=0.7, fillColor=circ_colour, children=[
                        dl.Tooltip(key),
                        dl.Popup(datetime.strftime(datetime.fromtimestamp(int(module_dict[key]['Timestamp'])), "%d-%m-%Y %H:%M:%S"))
                    ])
            )
    current_markers = marker
    clustered_markers = dl.MarkerClusterGroup(id="marker", children=marker, options={'maxClusterRadius': 30, 'showCoverageOnHover': False}) #'disableClusteringAtZoom': 6
    return clustered_markers

def add_lines(markers, makelines): #If makelines is true will return lines between points in order
    if makelines == False:
        return None 

    coords = []
    for marker in markers:
        coords.append(marker.center)
    lines = dl.Polyline(positions=coords, color="#00008B", weight = 3, opacity = 0.1, dashArray='20, 10') # #Add for dashed line
    design = [] #[dict(offset='100%', arrowHead=dict(pixelSize=80, polygon=False, pathOptions=dict(stroke=True)))] # add for arrow (Only at final point currently, unsure of how to make for every point at present)
    return dl.PolylineDecorator(children=lines, patterns=design) 

def bound_generator(latlongs): #Will return two latlongs that represent points that will contain all visable markers
    highest_lat = None
    div_ratio = 10

    for lat, long in latlongs:
        if highest_lat == None:
            highest_lat = lat
            highest_long = long
            lowest_lat = lat
            lowest_long = long
        if lat < lowest_lat:
            lowest_lat = lat
        elif lat > highest_lat:
            highest_lat = lat
        if long < lowest_long:
            lowest_long = long
        elif long > highest_long:
            highest_long = long
    
    long_pad = (highest_long - lowest_long) / div_ratio
    lat_pad = (highest_lat - lowest_lat) / div_ratio

    # lim = 90

    # if lowest_lat - lat_pad < -lim:
    #     lowest_lat = -lim + lat_pad
    # if highest_lat + lat_pad > lim:
    #     highest_lat = lim - lat_pad

    return [[lowest_lat-lat_pad, lowest_long-long_pad], [highest_lat+lat_pad, highest_long+long_pad]]

def module_sequencenum(module): #Used to establish the correct order of sequence numbers for circle markers table
    return int(module.split(splitter)[1])

def table_generate(ordered_modules, module_dict): #Used to create table and header
    table = [html.Div(id='tabhead', className='table_header', children=[
            html.Div(className='table_content_box', children=['ModuleID']),
            html.Div(className='table_content_box', children=['Timestamp']),
            html.Div(className='table_content_box', children=['Latitude']),
            html.Div(className='table_content_box', children=['Longitude'])
            ])]
    
    for module in ordered_modules:
        table = table + [html.Div(id = {'type': 'module_table', 'index': module}, className='table_unselected', children=[
            html.Div(className='table_content_box', children=[module_dict[module]['ModuleId']]),
            html.Div(className='table_content_box', children=[datetime.strftime(datetime.fromtimestamp(int(module_dict[module]['Timestamp'])), "%d-%m-%Y %H:%M")]), #:%S (Seconds if you want to reinclude)
            html.Div(className='table_content_box', children=[module_dict[module]['Latitude']]),
            html.Div(className='table_content_box', children=[module_dict[module]['Longitude']])
            ])]
    return table

def get_ordered_modules(module_dict, reorder): #Returns a list of 'moduleid splitter sequencenumber', in order of sequence number
    ordered_modules = []
    for module in module_dict:
            ordered_modules.append(module)

    if reorder == True: #If reorder was true, Apply reordering
        ordered_modules.sort(key=module_sequencenum, reverse=True)
    
    return ordered_modules

recent_markers = {} #Stores most recent markers avaliable (Will hold each modules most recent response)
current_markers = None
database = None
splitter = '|' #Defines what symbol will be used in unique module keys
last_search = None
# last_module_selected = None

database = scan_modules_database()

if not database:
    print('Critical error: Database not found!\nExiting....')
    exit()

original_dict = get_data(None)
original_order = get_ordered_modules(original_dict, False)
latlongs = []
for marker in original_dict:
    latlongs.append([original_dict[marker]['Latitude'], original_dict[marker]['Longitude']])

app = dash.Dash(__name__)
app.layout = html.Div( id='main_div', className='main_container',
        children = [
        html.Div(id='search_div', className='search_container', children = [
            dcc.Dropdown(
                id = 'search',
                value='',
                options=[{'label': name.split(splitter)[0], 'value': name.split(splitter)[0]} for name in original_dict],
                multi=False,
                placeholder='Search...'
                ),
            html.Div(id='search_items', className='module_container', children=table_generate(original_order, original_dict))
        ]),
        
        html.Div(id='map_div', className='map_container', children = [
            dl.Map(children=[
                    dl.TileLayer(id='tilelay'),
                    get_markers(original_order, original_dict)
                ],
                id='map',
                bounds = bound_generator(latlongs)),
            ])
])

@app.callback(
    Output(component_id='map', component_property='children'),
    Output(component_id='search_items', component_property='children'),
    Output(component_id='map', component_property='bounds'),
    Input(component_id='search', component_property='value'))
def search_marker_update(searchval):
    global last_search
    if searchval == last_search: #Avoids unnecessary update requests
        raise dash.exceptions.PreventUpdate
    else:
        last_search = searchval
    
    if searchval == '' or searchval == None:
        makelines = False
    else:
        makelines = True    
    
    module_dict = get_data(searchval)
    ordered_modules = get_ordered_modules(module_dict, makelines)
    markers = get_markers(ordered_modules, module_dict)
    
    new_tilelayer = [dl.TileLayer(id='tilelay'), markers, add_lines(current_markers, makelines)]

    latlongs = []

    for mar in markers.children:
        if mar.id['type'] == 'circle_marker':
            latlongs.append(list(mar.center))
        else:
            latlongs.append(list(mar.position))

    return new_tilelayer, table_generate(ordered_modules, module_dict), bound_generator(latlongs) 

@app.callback(
    Output(component_id='search', component_property='value'),
    Output({'type': 'module_table', 'index': ALL}, component_property='className'),
    Output({'type': 'circle_marker', 'index': ALL}, component_property='fillColor'),
    Input({'type': 'original_marker', 'index': ALL}, component_property='n_clicks'),
    Input({'type': 'module_table', 'index': ALL}, component_property='n_clicks'),
    Input({'type': 'circle_marker', 'index': ALL}, component_property='n_clicks'),
    State(component_id='search', component_property='value'),
    State({'type': 'module_table', 'index': ALL}, component_property='id'),
    State({'type': 'module_table', 'index': ALL}, component_property='className'),
    State({'type': 'circle_marker', 'index': ALL}, component_property='fillColor'), prevent_initial_callback = True)
def marker_select_update(*args):
    global last_module_selected
    mkr = dash.callback_context
    # orig_clicks = args[0]
    # table_clicks = args[1]
    # circ_clicks = args[2]
    # search_val = args[3]
    # table_values = args[4]
    # table_colors = args[5]
    # fill_colors = args[6]

    if 1 in args[0]: # If an original marker was clicked, change the search box value to the clicked event
        last_module_selected = mkr.triggered[0]['prop_id'].split('"')[3]
        return last_module_selected.split(splitter)[0], args[5], args[6] # table_clicks, table_clicks #Returns the moduleid of clicked marker (Does not include sequence number)
    elif 1 in args[1] or 1 in args[2]:
        last_module_selected = mkr.triggered[0]['prop_id'].split('"')[3]
        if args[3] != last_module_selected.split(splitter)[0]: #If we are not looking at this module, update the search box
            return last_module_selected.split(splitter)[0], args[5], args[6]  #orig_clicks, orig_clicks
        else: #If we reach here we are using the last triggered item to mark a selected module sequence.
            table_colors = []
            fill_colors = []
            for module in args[4]:
                if module['index'] == last_module_selected:
                    table_colors.append('table_selected') #Will need to fix in order to handle unusual ordering
                    fill_colors.append('#0077c8')
                else:
                    table_colors.append('table_unselected')
                    fill_colors.append('Yellow') 
            return args[3], table_colors, fill_colors
    else: #If no input was clicked, do not update
        raise dash.exceptions.PreventUpdate


if __name__ == '__main__':
    app.run_server(debug=True)



#========================================DELETE BELOW=========================