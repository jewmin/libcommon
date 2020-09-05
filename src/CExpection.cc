#include "CExpection.h"

namespace Common {

CExpection::CExpection() {
}

CExpection::CExpection(const i8 * message) : what_(message) {
}


CExpection::CExpection(CExpection && other) : what_(std::move(other.what_)) {
}

CExpection::CExpection(const CExpection & other) : what_(other.what_) {
}

CExpection & CExpection::operator=(CExpection && other) {
	if (this != std::addressof(other)) {
		what_ = std::move(other.what_);
	}
	return *this;
}

CExpection & CExpection::operator=(const CExpection & other) {
	if (this != std::addressof(other)) {
		what_ = other.what_;
	}
	return *this;
}

CExpection::~CExpection() {
}

const i8 * CExpection::What() const {
	return *what_;
}

}