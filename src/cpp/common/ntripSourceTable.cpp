
// #pragma GCC optimize ("O0")

#include "ntripSourceTable.hpp"


NtripSourceTable::NtripSourceTable(
	string url_str): NtripSocket(url_str)
{
	if (disconnectionCount > 2)
	{
		BOOST_LOG_TRIVIAL(warning) << url.sanitised()
								<< ", Could not connect to host for source table.\n";
		getSourceTableMtx.unlock();
		return;
	}
	
	std::stringstream	request_stream;
						request_stream	<< "GET / HTTP/1.1"										<< "\r\n";   
						request_stream	<< "Host: " 	<< url.host								<< "\r\n";
						request_stream	<< "Ntrip-Version: Ntrip/2.0"							<< "\r\n";   
	
	if (!url.user.empty())
	{	
						request_stream	<< "Authorization: Basic "
										<< Base64::encode(string(url.user + ":" + url.pass))	<< "\r\n";
	}
	
						request_stream	<< "User-Agent: NTRIP ACS/1.0"							<< "\r\n";   
						request_stream	<< "Connection: close"									<< "\r\n";   
						request_stream	<< "\r\n";
	request_string = request_stream.str();

	connect();
}

void NtripSourceTable::connected()
{  
	//BOOST_LOG_TRIVIAL(debug) << "Server Request  :\n" << request_string << std::endl;
	//BOOST_LOG_TRIVIAL(debug) << "Server Response :\n" << response_string << std::endl;
	
	string lineStr;
	std::stringstream responseStream(response_string);
	while (std::getline(responseStream, lineStr))
	{
		vector<string> tokens;
		boost::split(tokens, lineStr, boost::is_any_of(" \r\n"), boost::token_compress_on);
		
		if	(  tokens[0] == "Transfer-Encoding:" 
			&& tokens[1] == "chunked")
		{
			start_read(true);
			
			return;
		}
		
		if (tokens[0] == "Content-Length:")
		{
			try
			{
				content_length = std::stoi(tokens[1]);
			}
			catch (std::exception& e)
			{
				BOOST_LOG_TRIVIAL(error) << "Error, Server Response, NtripSourceTable::connected()\n";
				delayed_reconnect();  
				return;
			}
			
			start_read(false);
			
			return;
		}        
	}
	
	BOOST_LOG_TRIVIAL(error) << "Error, Server Response, NtripSourceTable::connected()\n";
	delayed_reconnect();
}

vector<string> NtripSourceTable::getStreamMounts()
{
	vector<string> mountPoints;
	
	for (auto entry : sourceTableData)
		mountPoints.push_back(entry.mountPoint);
	
	return mountPoints;
}


bool NtripSourceTable::dataChunkDownloaded(
	vector<char> dataChunk)
{
	sourceTableString.assign(dataChunk.begin(), dataChunk.end());
	

	//BOOST_LOG_TRIVIAL(debug) << sourceTableString;
	//BOOST_LOG_TRIVIAL(debug) << std::endl;

	getSourceTableMtx.unlock();
	return true;
}

void NtripSourceTable::readContentDownloaded(
	vector<char> content)
{
	sourceTableString.assign(content.begin(), content.end());
	
	//BOOST_LOG_TRIVIAL(debug) << sourceTableString;
	//BOOST_LOG_TRIVIAL(debug) << std::endl;

	getSourceTableMtx.unlock();    
}

void NtripSourceTable::getSourceTable()
{
	// Wait for the source table to read before program continues.
	//lock the mutex, try to lock it again - will fail until an async thread unlocks it, at which point we may continue
	getSourceTableMtx.lock();
	getSourceTableMtx.lock();
	getSourceTableMtx.unlock();
	
	// Get data on all mount points.
	string lineStr;
	string strBuf = sourceTableString;
	std::stringstream sourceTable(strBuf);
	
	while (std::getline(sourceTable, lineStr))
	{
		vector<string> tokens;
		boost::split(tokens, lineStr, boost::is_any_of(";\n"), boost::token_compress_on);
		
		string firstToken = tokens[0];
		boost::to_upper(firstToken);
		
		if (firstToken != "STR")
			continue;
		
		// Not all source table data was included as it is yet to be required.
		SourceTableEntry curEntry;
		curEntry.mountPoint	= tokens[1];
		curEntry.location	= tokens[2];
		
		vector<string> messagesTokens;
		boost::split(messagesTokens, tokens[4], boost::is_any_of(","), boost::token_compress_on);
		for (int i = 0; i < messagesTokens.size(); i++)
		{
			int RtcmType;
			int frequencySeconds;
			vector<string> messTokens;
			boost::split(messTokens, messagesTokens[i], boost::is_any_of("()"), boost::token_compress_on);
			try
			{            
				RtcmType = std::stoi(messTokens[0]);
				
				frequencySeconds = -1;
				
				if (messTokens.size() == 2)
					frequencySeconds = std::stoi(messTokens[1]);
				
				SourceTableEntry::RtcmMessageData messData;     

				//messData.messageType = RtcmMessageType::_from_integral(RtcmType);
				messData.messageType			= RtcmType;
				messData.broadcastFrequency		= frequencySeconds;
				
				curEntry.messageData.push_back(messData);                
			}    
			catch (std::exception& e)
			{
// 				BOOST_LOG_TRIVIAL(error) << "Error reading source table.\n";
// 				BOOST_LOG_TRIVIAL(error) << "URL : " << url.sanitised() << std::endl;
// 				BOOST_LOG_TRIVIAL(error) << "SOURCETABLE line : " << lineStr << std::endl;
			}
		}
		sourceTableData.push_back(curEntry);
	}
}
