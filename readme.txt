What is done?
-Every test except testIllum.txt is working perfectly
-testIllum.txt has a small error in the diffuse lighting on the inside of the truncated sphere

What is omitted?
-Nothing is omitted

Compilation/running instructions:
My project is included as a Visual Studio solution. I have confirmed that you can compile it on the lab computers using Visual Studio 2019.
1. Extract the ZIP file. Inside there should be folders "RayTracer" and "tests" and file "readme.txt" (the one you're currently reading).
2. In Visual Studio 2019, open the project solution (RayTracer/RayTracer.sln).
3. Build using CTRL+Shift+B or use Build->Build Solution in the top bar.
4. The executable for the project will automatically be outputted to tests/RayTracer.exe. The tests folder contains all of the tests for the project plus the keys. It also includes a short python program which can run all of the tests at once. To use it, type "python runner.py".
5. To run the RayTracer directly, type "RayTracer.exe <testfile>" exactly like how the assignment description specifies.

Note: I have included a compiled binary that was built on my machine and I've verified that it also works on the lab in case you have any troubles.
