import { AcademicLevel, StudentStatus } from '../entities/student.entity';

export class StudentResponseDto {
  id: string;
  userId: string;
  studentNumber: string;
  academicLevel: AcademicLevel;
  status: StudentStatus;
  enrollmentDate: Date;
  specialization?: string;
  gpa?: number;
  totalCredits: number;
  emergencyContactName?: string;
  emergencyContactPhone?: string;
  createdAt: Date;
  updatedAt: Date;
}
