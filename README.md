# QubicleBinary
Implementation of Qubicle Binary in UnrealEngine 4

More information on the format can be found here: [Qubicle Binary Data Exchange Format](http://minddesk.com/wiki/index.php?title=Qubicle_Constructor_1:Data_Exchange_With_Qubicle_Binary)

Basic usage:
Implement the `IQubicleBinaryAccessor` interface

Then on that object call `FQubicleBinaryReader::ReadFileIntoObject(FileName, dynamic_cast<IQubicleBinaryAccessor*>(this));`
