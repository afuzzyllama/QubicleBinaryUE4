# QubicleBinaryReader
Implementation of Qubicle Binary in UnrealEngine 4

More information on the format can be found here:
http://minddesk.com/wiki/index.php?title=Qubicle_Constructor_1:Data_Exchange_With_Qubicle_Binary

Basic usage:
Implement the `IQubicleBinaryAccessor` interface

Then on that object call `QubicleBinaryReader::ReadIntoObject(QubicleBinaryData, ObjWithInterface);`
