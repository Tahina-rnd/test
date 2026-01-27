import {
  Injectable,
  NotFoundException,
  ConflictException,
} from '@nestjs/common';
import { InjectRepository } from '@nestjs/typeorm';
import { Repository } from 'typeorm';
import { Student } from './entities/student.entity';
import { User } from '../users/entities/user.entity';
import { CreateStudentDto } from './dto/create-student.dto';
import { UpdateStudentDto } from './dto/update-student.dto';
import { StudentResponseDto } from './dto/student-response.dto';

@Injectable()
export class StudentsService {
  constructor(
    @InjectRepository(Student)
    private studentRepository: Repository<Student>,
    @InjectRepository(User)
    private userRepository: Repository<User>,
  ) {}

  /**
   * Create a new student.
   *
   * @param createStudentDto - The student data to create.
   *
   * @returns A promise that resolves to the created student.
   *
   * @throws ConflictException - If the student number is already used.
   * @throws NotFoundException - If the user is not found.
   */
  async create(
    createStudentDto: CreateStudentDto,
  ): Promise<StudentResponseDto> {
    // Check if user exists
    const user = await this.userRepository.findOne({
      where: { id: createStudentDto.userId },
    });

    if (!user) {
      throw new NotFoundException(
        `User with id ${createStudentDto.userId} not found`,
      );
    }

    // Check if student number is already used
    const existingStudent = await this.studentRepository.findOne({
      where: { studentNumber: createStudentDto.studentNumber },
    });

    if (existingStudent) {
      throw new ConflictException('Student number already exists');
    }

    const student = this.studentRepository.create({
      ...createStudentDto,
      enrollmentDate: new Date(createStudentDto.enrollmentDate),
    });

    const savedStudent = await this.studentRepository.save(student);
    return this.mapToResponseDto(savedStudent);
  }

  /**
   * Find all students.
   *
   * @returns A promise that resolves to an array of students.
   */
  async findAll(): Promise<StudentResponseDto[]> {
    const students = await this.studentRepository.find({
      relations: ['user'],
    });
    return students.map((student) => this.mapToResponseDto(student));
  }

  /**
   * Find a student by id.
   *
   * @param id - The id of the student to find.
   *
   * @returns A promise that resolves to the student.
   *
   * @throws NotFoundException - If the student is not found.
   */
  async findOne(id: string): Promise<StudentResponseDto> {
    const student = await this.studentRepository.findOne({
      where: { id },
      relations: ['user'],
    });

    if (!student) {
      throw new NotFoundException(`Student with id ${id} not found`);
    }

    return this.mapToResponseDto(student);
  }

  /**
   * Find a student by user id.
   *
   * @param userId - The user id of the student to find.
   *
   * @returns A promise that resolves to the student.
   *
   * @throws NotFoundException - If the student is not found.
   */
  async findByUserId(userId: string): Promise<StudentResponseDto> {
    const student = await this.studentRepository.findOne({
      where: { userId },
      relations: ['user'],
    });

    if (!student) {
      throw new NotFoundException(`Student with user id ${userId} not found`);
    }

    return this.mapToResponseDto(student);
  }

  /**
   * Update a student.
   *
   * @param id - The id of the student to update.
   * @param updateStudentDto - The student data to update.
   *
   * @returns A promise that resolves to the updated student.
   *
   * @throws NotFoundException - If the student is not found.
   * @throws ConflictException - If the student number is already used by another student.
   */
  async update(
    id: string,
    updateStudentDto: UpdateStudentDto,
  ): Promise<StudentResponseDto> {
    const student = await this.studentRepository.findOne({
      where: { id },
    });

    if (!student) {
      throw new NotFoundException(`Student with id ${id} not found`);
    }

    // Check if student number is already used by another student
    if (
      updateStudentDto.studentNumber !== undefined &&
      updateStudentDto.studentNumber !== student.studentNumber
    ) {
      const existingStudent = await this.studentRepository.findOne({
        where: { studentNumber: updateStudentDto.studentNumber },
      });

      if (existingStudent) {
        throw new ConflictException('Student number already exists');
      }
    }

    const updatedStudent = await this.studentRepository.save({
      ...student,
      ...updateStudentDto,
      enrollmentDate:
        updateStudentDto.enrollmentDate !== undefined
          ? new Date(updateStudentDto.enrollmentDate)
          : student.enrollmentDate,
    });

    return this.mapToResponseDto(updatedStudent);
  }

  /**
   * Remove a student (soft delete).
   *
   * @param id - The id of the student to remove.
   *
   * @throws NotFoundException - If the student is not found.
   */
  async remove(id: string): Promise<void> {
    const student = await this.studentRepository.findOne({
      where: { id },
    });

    if (!student) {
      throw new NotFoundException(`Student with id ${id} not found`);
    }

    await this.studentRepository.softDelete(id);
  }

  /**
   * Map a Student entity to StudentResponseDto.
   *
   * @param student - The student entity to map.
   *
   * @returns The mapped StudentResponseDto.
   */
  private mapToResponseDto(student: Student): StudentResponseDto {
    return {
      id: student.id,
      userId: student.userId,
      studentNumber: student.studentNumber,
      academicLevel: student.academicLevel,
      status: student.status,
      enrollmentDate: student.enrollmentDate,
      specialization: student.specialization,
      gpa: student.gpa,
      totalCredits: student.totalCredits,
      emergencyContactName: student.emergencyContactName,
      emergencyContactPhone: student.emergencyContactPhone,
      createdAt: student.createdAt,
      updatedAt: student.updatedAt,
    };
  }
}
