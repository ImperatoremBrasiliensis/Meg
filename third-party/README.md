# Third-Party Projects Folder

This folder contains open-source projects used in the Meg Project for development and testing purposes only. These projects are not part of the distribution.

**note**: 
All projects in this folder are used internally for code development, specifically for code testing, maintenance, and stability assurance.

# Projects

## GoogleTest & GoogleMock (v1.17.0)

**GoogleTest** and **GoogleMock** are open-source projects developed by Google for creating unit tests and mocking objects in C++.

- **GoogleTest**: Located in `googletest/`
- **GoogleMock**: Located in `googlemock/`
- **Source**: [Google Test Repository](https://github.com/google/googletest/releases/tag/v1.17.0)
- **License**: BSD 3-Clause License
- **Purpose**: Unit testing framework and mocking library for the Meg Project

### Customizations

Both projects have been customized for this project. For detailed information about the changes made, please refer to:

- [googletest/README.md](googletest/README.md)
- [googlemock/README.md](googlemock/README.md)

### Licensing

#### Dependencies Used Only for Testing
Since GoogleTest and GoogleMock are used only for testing and development (not distributed with the final product), their BSD 3-Clause License is compatible with the Meg Project's GPLv3 license in the development environment.

#### License Files
- GoogleTest License: [googletest/LICENSE](googletest/LICENSE)
- GoogleMock License: [googlemock/LICENSE](googlemock/LICENSE)

---

*We acknowledge the open-source community for making projects like the Meg Project possible, enabling developers worldwide to build robust and reliable software.*
