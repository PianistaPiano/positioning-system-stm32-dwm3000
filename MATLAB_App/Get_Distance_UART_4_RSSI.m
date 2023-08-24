function [distances, rssi,error, tag, readDistAndRssi] = Get_Distance_UART_4_RSSI(device)
    % read Data in format: ex. "0.04/-45.67:0.62/-67.08:0.42/-59.43:0.67/-80.31:0.99/-77.43:0.78/-81.45:E"
    %                               A      :     B     :    C      :    D      :    E      :    F      : Tag ID
    %                           "Dist/RSSI : Dist/RSSI : ..."
    error_flag = 0;
    rssi = [];
    distances = [];
    try
            readData = (readline(device));
            % split data using ":"
            splitData = split(readData, ":");
            % convert to double, splitData(1:end-1) because last split is tag
            % type
    %       distancesAndRssi = str2double(splitData(1:end-1));
    %       distances = split(distancesAndRssi, "/");
            for i=1:length(splitData) - 1
                splitTemp = split(splitData(i), "/");
                distances(i) = str2double(splitTemp(1));
                rssi(i) = str2double(splitTemp(2));
            end
            tag = convertStringsToChars(splitData(end));
            tag = tag(1);
            readDistAndRssi = readData;
    catch
            error_flag = 1;
    end %try catch
    
    if(error_flag)
        distances = 0;
        error = 1;
        tag = 0;
    else
        error = 0;
    end % if

end % function